//
// Created by eharquin on 12/7/25.
//


#include <core/rendering/vulkan/Context.hpp>

#include <algorithm>
#include <stdexcept>
#include <iostream>
#include <string>
#include <map>

#define GLFW_INCLUDE_VULKAN
#include <numeric>
#include <GLFW/glfw3.h>

#include <core/utils/FileUtils.hpp>
#include <core/utils/ImageUtils.hpp>

#define TINYOBJLOADER_IMPLEMENTATION

namespace Core::Rendering::Vulkan {

	void Context::create(const Window & window) {
		createInstance();
		setupDebugMessenger();
		createSurface(window);
		pickPhysicalDevice();
		createLogicalDevice();

		int width = window.width();
		int height = window.height();

		createSwapChain(width,height);
		createImageViews();
		createCommandPool();
		createDescriptorSetLayout();
		createGraphicsPipeline();
		createDepthResources();
		createTextureImage();
		createTextureImageView();
		createTextureSampler();
		loadModel();
		createVertexBuffer();
		createIndexBuffer();
		createUniformBuffers();
		createDescriptorPool();
		createDescriptorSets();
		createCommandBuffers();
		createSyncObjects();
	}

	void Context::drawFrame(int width, int height) {

		// Note: inFlightFences, presentCompleteSemaphores, and commandBuffers are indexed by frameIndex,
		//       while renderFinishedSemaphores is indexed by imageIndex
		while (vk::Result::eTimeout == _device.waitForFences(*_inFlightFences[_frameIndex], vk::True, UINT64_MAX));
		_device.resetFences(*_inFlightFences[_frameIndex]);

		auto [result, imageIndex] = _swapChain.acquireNextImage( UINT64_MAX, *_presentCompleteSemaphores[_frameIndex], nullptr );

		if (result == vk::Result::eErrorOutOfDateKHR) {
			recreateSwapChain(width, height);
			std::cout << "Swap chain recreated due to eErrorOutOfDateKHR during acquireNextImage.\n";
			return;
		}
		if (result != vk::Result::eSuccess && result != vk::Result::eSuboptimalKHR) {
			throw std::runtime_error("failed to acquire swap chain image!");
		}


		updateUniformBuffer(_frameIndex);

		_commandBuffers[_frameIndex].reset();
		recordCommandBuffer(imageIndex);

		vk::PipelineStageFlags waitDestinationStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput);
		const vk::SubmitInfo submitInfo{
			.waitSemaphoreCount = 1,
			.pWaitSemaphores = &*_presentCompleteSemaphores[_frameIndex],
			.pWaitDstStageMask = &waitDestinationStageMask,
			.commandBufferCount = 1,
			.pCommandBuffers = &*_commandBuffers[_frameIndex],
			.signalSemaphoreCount = 1,
			.pSignalSemaphores = &*_renderFinishedSemaphores[imageIndex]
		};

		_graphicsQueue.submit(submitInfo, *_inFlightFences[_frameIndex]);

		try
		{
			const vk::PresentInfoKHR presentInfoKHR{.waitSemaphoreCount = 1,
													.pWaitSemaphores    = &*_renderFinishedSemaphores[imageIndex],
													.swapchainCount     = 1,
													.pSwapchains        = &*_swapChain,
													.pImageIndices      = &imageIndex};
			result = _graphicsQueue.presentKHR(presentInfoKHR);
			if (result == vk::Result::eSuboptimalKHR || _shouldRecreateSwapChain)
			{
				std::cout << "Swap chain recreated due to eSuboptimalKHR or window resize during presentKHR.\n";
				_shouldRecreateSwapChain = false;
				recreateSwapChain(width, height);
			}
			else if (result != vk::Result::eSuccess)
				throw std::runtime_error("failed to present swap chain image!");
		}
		catch (const vk::SystemError &e)
		{
			if (e.code().value() == static_cast<int>(vk::Result::eErrorOutOfDateKHR))
			{
				std::cout << "Swap chain recreated due to eErrorOutOfDateKHR exception during presentKHR.\n";
				recreateSwapChain(width, height);
				return;
			}
			else
			{
				throw;
			}
		}

		_frameIndex = (_frameIndex + 1) % MAX_FRAMES_IN_FLIGHT;
	}

	void Context::stop() {
		_device.waitIdle();
	}

	// region Instance Creation
	void Context::createInstance() {
		constexpr vk::ApplicationInfo appInfo{
			.pApplicationName = "Hello Triangle",
			.applicationVersion = VK_MAKE_VERSION(1, 0, 0),
			.pEngineName = "No Engine",
			.engineVersion = VK_MAKE_VERSION(1, 0, 0),
			.apiVersion = vk::ApiVersion14
		};

		std::vector<const char *> requiredLayers;
		if (enableValidationLayers)
			requiredLayers.assign(validationLayers.begin(), validationLayers.end());
		if (!checkLayerSupport(requiredLayers))
			throw std::runtime_error("Required layers not supported");

		auto requiredExtensions = getRequiredExtensions();
		if (!checkExtensionSupport(requiredExtensions))
			throw std::runtime_error("Required extensions not supported");

		const vk::InstanceCreateInfo createInfo{
			.pApplicationInfo = &appInfo,
			.enabledLayerCount = static_cast<uint32_t>(requiredLayers.size()),
			.ppEnabledLayerNames = requiredLayers.data(),
			.enabledExtensionCount = static_cast<uint32_t>(requiredExtensions.size()),
			.ppEnabledExtensionNames = requiredExtensions.data()
		};

		_instance = vk::raii::Instance(_context, createInfo);
		setupDebugMessenger();
	}

	std::vector<const char *> Context::getRequiredExtensions() {
		// Get GLFW extensions needed
		uint32_t glfwExtensionCount = 0;
		const auto glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

		std::vector<const char *> extensions{
			glfwExtensions,
			glfwExtensions + glfwExtensionCount
		};

		// Add EXT_DEBUG_UTILS extensions IF Validation Layers enable
		if (enableValidationLayers)
			extensions.push_back(vk::EXTDebugUtilsExtensionName);
		return extensions;
	}

	bool Context::checkExtensionSupport(const std::vector<const char *> &requiredExtensions) const {
		bool foundAll = true;
		// Check if the required extensions are supported by the Vulkan implementation.
		auto extensionProperties = _context.enumerateInstanceExtensionProperties();
		for (auto requiredExtension: requiredExtensions) {
			if (std::ranges::none_of(extensionProperties,
			                         [requiredExtension](auto const &ext) {
				                         return std::string_view(ext.extensionName) == requiredExtension;
			                         })) {
				foundAll = false;
				std::cerr << "[ASTRO CORE] [VULKAN] [CHECK] extension NOK: " + std::string(requiredExtension) <<
						std::endl;
			} else {
				std::cout << "[ASTRO CORE] [VULKAN] [CHECK] extension  OK: " + std::string(requiredExtension) <<
						std::endl;
			}
		}

		return foundAll;
	}


	bool Context::checkLayerSupport(const std::vector<const char *> &requiredLayers) const {
		bool foundAll = true;
		// Check if the required instance layers are supported by the Vulkan implementation.
		auto layerProperties = _context.enumerateInstanceLayerProperties();
		for (auto requiredLayer: requiredLayers) {
			if (std::ranges::none_of(layerProperties,
			                         [requiredLayer](auto const &ext) {
				                         return std::string_view(ext.layerName) == requiredLayer;
			                         })) {
				foundAll = false;
				std::cerr << "[ASTRO CORE] [VULKAN] [CHECK] layer     NOK: " + std::string(requiredLayer) <<
						std::endl;
			} else {
				std::cout << "[ASTRO CORE] [VULKAN] [CHECK] layer      OK: " + std::string(requiredLayer) << std::endl;
			}
		}

		return foundAll;
	}

	// endregion

	// region Debug Messenger
	vk::Bool32 Context::debugCallback(vk::DebugUtilsMessageSeverityFlagBitsEXT severity,
	                                  vk::DebugUtilsMessageTypeFlagsEXT type,
	                                  const vk::DebugUtilsMessengerCallbackDataEXT *pCallbackData, void *) {
		std::cerr << "[VALIDATION LAYER]  type: " << to_string(type) << " msg: " << pCallbackData->pMessage <<
				std::endl;
		return vk::False;
	}

	void Context::setupDebugMessenger() {
		constexpr vk::DebugUtilsMessageSeverityFlagsEXT severityFlags(
			vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose | vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
			vk::DebugUtilsMessageSeverityFlagBitsEXT::eError);

		constexpr vk::DebugUtilsMessageTypeFlagsEXT messageTypeFlags(
			/*vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | */
			vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance |
			vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation);


		constexpr vk::DebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCreateInfoEXT{
			.messageSeverity = severityFlags,
			.messageType = messageTypeFlags,
			.pfnUserCallback = &debugCallback
		};

		_debugMessenger = _instance.createDebugUtilsMessengerEXT(debugUtilsMessengerCreateInfoEXT);
	}


	// endregion

	// region Device
	void Context::pickPhysicalDevice() {
		if (_instance == nullptr)
			throw std::runtime_error("Vulkan instance not created");

		auto devices = vk::raii::PhysicalDevices(_instance);
		if (devices.empty()) {
			throw std::runtime_error("failed to find GPUs with Vulkan support!");
		}
		const auto devIter =
				std::ranges::find_if(devices, [&](auto const &device) {
					// Check for Vulkan 1.3 support
					bool isSuitable = device.getProperties().apiVersion >= VK_API_VERSION_1_4;

					std::cout << "[ASTRO CORE] [VULKAN] [CHECK] device       : " <<
							device.getProperties().deviceName << " API Version: " <<
							VK_VERSION_MAJOR(device.getProperties().apiVersion) << "." <<
							VK_VERSION_MINOR(device.getProperties().apiVersion) << "." <<
							VK_VERSION_PATCH(device.getProperties().apiVersion) <<
							(isSuitable ? "  OK" : "  NOK") << std::endl;

					// Check for graphics queue support
					auto queueFamilies = device.getQueueFamilyProperties();
					const auto qfpIter =
							std::ranges::find_if(queueFamilies, [](vk::QueueFamilyProperties const &qfp) {
								return (qfp.queueFlags & vk::QueueFlagBits::eGraphics) != static_cast<vk::QueueFlags>(
									       0);
							});

					std::cout << "[ASTRO CORE] [VULKAN] [CHECK] graphics queue   : " <<
							(qfpIter != queueFamilies.end() ? "FOUND" : "NOT FOUND") << std::endl;

					isSuitable = isSuitable && (qfpIter != queueFamilies.end());

					// Check for device extension support
					auto extensions = device.enumerateDeviceExtensionProperties();
					bool found = true;
					for (auto const &extension: deviceExtensions) {
						auto extensionIter =
								std::ranges::find_if(extensions, [extension](auto const &ext) {
									return strcmp(ext.extensionName, extension) == 0;
								});
						found = found && extensionIter != extensions.end();

						std::cout << "[ASTRO CORE] [VULKAN] [CHECK] device extension: " +
								std::string(extension) + (extensionIter != extensions.end() ? "  OK" : "  NOK")
								<< std::endl;
					}

					auto features = device.template getFeatures2<vk::PhysicalDeviceFeatures2,
													  vk::PhysicalDeviceVulkan11Features,
													  vk::PhysicalDeviceVulkan13Features,
													  vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>();

					bool supportsRequiredFeatures = features.template get<vk::PhysicalDeviceVulkan11Features>().shaderDrawParameters &&
													features.template get<vk::PhysicalDeviceVulkan13Features>().synchronization2 &&
													features.template get<vk::PhysicalDeviceVulkan13Features>().dynamicRendering &&
													features.template get<vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>().extendedDynamicState &&
													features.template get<vk::PhysicalDeviceFeatures2>().features.samplerAnisotropy;

					isSuitable = isSuitable && found && supportsRequiredFeatures;

					if (isSuitable)
						_physicalDevice = device;

					return isSuitable;
				});
		if (devIter == devices.end()) {
			throw std::runtime_error("failed to find a suitable GPU!");
		}
	}


	Context::QueueFamilyIndices Context::findQueueFamilies(const vk::raii::PhysicalDevice &physicalDevice, const vk::raii::SurfaceKHR &surface) {

		const auto queueFamilyProperties = physicalDevice.getQueueFamilyProperties();

		QueueFamilyIndices indices;
		for (uint32_t i = 0; i < queueFamilyProperties.size(); ++i) {
			if (const auto &qfp = queueFamilyProperties[i]; (qfp.queueFlags & vk::QueueFlagBits::eGraphics)) indices.graphicsFamily = i;
			if (physicalDevice.getSurfaceSupportKHR(i, *surface)) indices.presentFamily = i;
			if (indices.isComplete()) break;
		}


		if (!indices.isComplete())
			throw std::runtime_error("failed to find required queue families!");

		std::cout << "[ASTRO CORE] [VULKAN] [CHECK] graphics family : " <<
				(indices.graphicsFamily.has_value() ? std::to_string(indices.graphicsFamily.value()) : "NOT FOUND")
				<< std::endl;
		std::cout << "[ASTRO CORE] [VULKAN] [CHECK] present family  : " <<
				(indices.presentFamily.has_value() ? std::to_string(indices.presentFamily.value()) : "NOT FOUND")
				<< std::endl;

		return indices;
	}

	void Context::createLogicalDevice() {
		std::vector<vk::QueueFamilyProperties> queueFamilyProperties = _physicalDevice.getQueueFamilyProperties();
		_queueFamilyIndices = findQueueFamilies(_physicalDevice, _surface);
		float queuePriority = 0.5f;

		vk::DeviceQueueCreateInfo deviceQueueCreateInfo{
			.queueFamilyIndex = _queueFamilyIndices.graphicsFamily.value(),
			.queueCount = 1,
			.pQueuePriorities = &queuePriority
		};

		// TO COMPLETE FURTHER
		vk::PhysicalDeviceFeatures deviceFeatures;

		// Create a chain of feature structures
		vk::StructureChain<vk::PhysicalDeviceFeatures2,
		vk::PhysicalDeviceVulkan11Features,
		vk::PhysicalDeviceVulkan13Features,
		vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT> featureChain = {
			{.features = {.samplerAnisotropy = true } }, // vk::PhysicalDeviceFeatures2
			{.shaderDrawParameters = true},        // vk::PhysicalDeviceVulkan11Features
			{.synchronization2 = true, .dynamicRendering = true},            // vk::PhysicalDeviceVulkan13Features
			{.extendedDynamicState = true}        // vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT
		};

		vk::DeviceCreateInfo deviceCreateInfo{
			.pNext = &featureChain.get<vk::PhysicalDeviceFeatures2>(),
			.queueCreateInfoCount = 1,
			.pQueueCreateInfos = &deviceQueueCreateInfo,
			.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size()),
			.ppEnabledExtensionNames = deviceExtensions.data()
		};

		_device = vk::raii::Device(_physicalDevice, deviceCreateInfo);
		_graphicsQueue = vk::raii::Queue(_device, _queueFamilyIndices.graphicsFamily.value(), 0);
		_presentQueue = vk::raii::Queue(_device, _queueFamilyIndices.presentFamily.value(), 0);
	}


	// endregion

	// region Surface
	void Context::createSurface(const Window & window) {
		VkSurfaceKHR       surface0;
		if (glfwCreateWindowSurface(*_instance, window.glfwHandle(), nullptr, &surface0) != 0) {
			throw std::runtime_error("failed to create window surface!");
		}
		_surface = vk::raii::SurfaceKHR(_instance, surface0);
	}
	// endregion

	// region Swap Chain
	void Context::createSwapChain(int width, int height) {
		auto chooseSwapSurfaceFormat = [](const std::vector<vk::SurfaceFormatKHR>& availableFormats) {
			for (const auto& f : availableFormats) {
				if (f.format == vk::Format::eB8G8R8A8Srgb &&
					f.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) return f;
			}
			return availableFormats[0];
		};
		auto chooseSwapPresentMode = [](const std::vector<vk::PresentModeKHR>& availablePresentModes) {
			for (const auto& mode : availablePresentModes) {
				if (mode == vk::PresentModeKHR::eMailbox) return mode;
			}
			return vk::PresentModeKHR::eFifo;
		};
		auto chooseSwapExtent = [width, height](const vk::SurfaceCapabilitiesKHR& capabilities) {
			if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) return capabilities.currentExtent;

			return vk::Extent2D{
				std::clamp<uint32_t>(width,  capabilities.minImageExtent.width,  capabilities.maxImageExtent.width),
				std::clamp<uint32_t>(height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height)
			};
		};
		auto chooseSwapMinImageCount = [](const vk::SurfaceCapabilitiesKHR& surfaceCapabilities) {
			auto minImageCount = std::max(3u, surfaceCapabilities.minImageCount);
			if ((surfaceCapabilities.maxImageCount > 0) && (surfaceCapabilities.maxImageCount < minImageCount)) {
				minImageCount = surfaceCapabilities.maxImageCount;
			}
			return minImageCount;
		};

		const auto surfaceCapabilities = _physicalDevice.getSurfaceCapabilitiesKHR( _surface );
		_swapChainSurfaceFormat = chooseSwapSurfaceFormat( _physicalDevice.getSurfaceFormatsKHR( _surface ) );
		_swapChainPresentMode = chooseSwapPresentMode( _physicalDevice.getSurfacePresentModesKHR( _surface ) );
		_swapChainExtent = chooseSwapExtent( surfaceCapabilities );
		_swapChainMinImageCount = chooseSwapMinImageCount( surfaceCapabilities );

		std::cout << "[ASTRO CORE] [VULKAN] [SWAPCHAIN] surface format: " <<
			to_string(_swapChainSurfaceFormat.format) << " " <<
			to_string(_swapChainSurfaceFormat.colorSpace) << std::endl;
		std::cout << "[ASTRO CORE] [VULKAN] [SWAPCHAIN] present mode  : " <<
			to_string(_swapChainPresentMode) << std::endl;
		std::cout << "[ASTRO CORE] [VULKAN] [SWAPCHAIN] extent        : " <<
			_swapChainExtent.width << "x" << _swapChainExtent.height << std::endl;
		std::cout << "[ASTRO CORE] [VULKAN] [SWAPCHAIN] min image count: " <<
			_swapChainMinImageCount << std::endl;

		vk::SwapchainCreateInfoKHR swapChainCreateInfo{
			.surface          = *_surface,
		   .minImageCount    = _swapChainMinImageCount,
		   .imageFormat      = _swapChainSurfaceFormat.format,
		   .imageColorSpace  = _swapChainSurfaceFormat.colorSpace,
		   .imageExtent      = _swapChainExtent,
		   .imageArrayLayers = 1,
		   .imageUsage       = vk::ImageUsageFlagBits::eColorAttachment,
		   .imageSharingMode = vk::SharingMode::eExclusive,
		   .preTransform     = surfaceCapabilities.currentTransform,
		   .compositeAlpha   = vk::CompositeAlphaFlagBitsKHR::eOpaque,
		   .presentMode      = _swapChainPresentMode,
		   .clipped          = true};

		_swapChain = vk::raii::SwapchainKHR(_device, swapChainCreateInfo);
		_swapChainImages = _swapChain.getImages();
	}

	void Context::cleanupSwapChain() {
		_swapChainImageViews.clear();
		_swapChain = nullptr;
	}

	void Context::recreateSwapChain(int width, int height) {
		_device.waitIdle();

		cleanupSwapChain();

		createSwapChain(width,height);
		createImageViews();
		createDepthResources();
	}

	void Context::shouldRecreateSwapChain() {
		_shouldRecreateSwapChain = true;
	}

	// endregion

	// region Image Views
	void Context::createImageViews() {
		assert(_swapChainImageViews.empty());

		vk::ImageViewCreateInfo imageViewCreateInfo{.viewType = vk::ImageViewType::e2D, .format = _swapChainSurfaceFormat.format, .subresourceRange = {vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1}};
		for (auto& image : _swapChainImages)
		{
			imageViewCreateInfo.image = image;
			_swapChainImageViews.emplace_back(_device, imageViewCreateInfo);
		}
	}
	// endregion

	// region Descriptor Set Layout
	void Context::createDescriptorSetLayout() {
		std::array bindings = {
			vk::DescriptorSetLayoutBinding( 0, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eVertex, nullptr),
			vk::DescriptorSetLayoutBinding( 1, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment, nullptr)
		};

		vk::DescriptorSetLayoutCreateInfo layoutInfo{.bindingCount = bindings.size(), .pBindings = bindings.data()};
		_descriptorSetLayout = vk::raii::DescriptorSetLayout(_device, layoutInfo);
	}

	// endregion

	// region Graphics Pipeline

	vk::raii::ShaderModule Context::createShaderModule(const std::vector<char> &code) const {
		const vk::ShaderModuleCreateInfo createInfo{
			.codeSize = code.size() * sizeof(char),
			.pCode = reinterpret_cast<const uint32_t*>(code.data())
		};
		return { _device, createInfo };
	}

	void Context::createGraphicsPipeline() {
		const vk::raii::ShaderModule shaderModule = createShaderModule(Utils::readFile("../../shaders/slang.spv"));

		vk::PipelineShaderStageCreateInfo vertShaderStageInfo{
			.stage = vk::ShaderStageFlagBits::eVertex,
			.module = shaderModule,
			.pName = "vertMain"
		};

		vk::PipelineShaderStageCreateInfo fragShaderStageInfo{
			.stage = vk::ShaderStageFlagBits::eFragment,
			.module = shaderModule,
			.pName = "fragMain"
		};

		vk::PipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

		auto bindingDescription = Vertex::getBindingDescription();
		auto attributeDescriptions = Vertex::getAttributeDescriptions();
		vk::PipelineVertexInputStateCreateInfo vertexInputInfo {
			.vertexBindingDescriptionCount =1,
			.pVertexBindingDescriptions = &bindingDescription,
			.vertexAttributeDescriptionCount = attributeDescriptions.size(),
			.pVertexAttributeDescriptions = attributeDescriptions.data()
		};

		vk::PipelineInputAssemblyStateCreateInfo inputAssembly{.topology = vk::PrimitiveTopology::eTriangleList};
		vk::PipelineViewportStateCreateInfo      viewportState{.viewportCount = 1, .scissorCount = 1};

		vk::PipelineRasterizationStateCreateInfo rasterizer{
			.depthClampEnable = vk::False,
			.rasterizerDiscardEnable = vk::False,
			.polygonMode = vk::PolygonMode::eFill,
			.cullMode = vk::CullModeFlagBits::eBack,
			.frontFace = vk::FrontFace::eCounterClockwise,
			.depthBiasEnable = vk::False,
			.depthBiasSlopeFactor = 1.0f,
			.lineWidth = 1.0f};

		vk::PipelineMultisampleStateCreateInfo multisampling{
			.rasterizationSamples = vk::SampleCountFlagBits::e1,
			.sampleShadingEnable = vk::False};

		vk::PipelineColorBlendAttachmentState colorBlendAttachment{
			.blendEnable    = vk::False,
			.colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA};

		vk::PipelineColorBlendStateCreateInfo colorBlending{
			.logicOpEnable = vk::False,
			.logicOp = vk::LogicOp::eCopy,
			.attachmentCount = 1,
			.pAttachments = &colorBlendAttachment};

		std::vector dynamicStates = {
			vk::DynamicState::eViewport,
			vk::DynamicState::eScissor};

		vk::PipelineDynamicStateCreateInfo dynamicState{
			.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size()),
			.pDynamicStates = dynamicStates.data()};

		vk::PipelineLayoutCreateInfo pipelineLayoutInfo{
			.setLayoutCount = 1,
			.pSetLayouts = &*_descriptorSetLayout,
			.pushConstantRangeCount = 0
		};

		_pipelineLayout = vk::raii::PipelineLayout(_device, pipelineLayoutInfo);

		vk::PipelineDepthStencilStateCreateInfo depthStencil{
			.depthTestEnable       = vk::True,
			.depthWriteEnable      = vk::True,
			.depthCompareOp        = vk::CompareOp::eLess,
			.depthBoundsTestEnable = vk::False,
			.stencilTestEnable     = vk::False};

		vk::Format depthFormat = findDepthFormat();

		vk::StructureChain<vk::GraphicsPipelineCreateInfo, vk::PipelineRenderingCreateInfo> pipelineCreateInfoChain = {
			{.stageCount          = 2,
			 .pStages             = shaderStages,
			 .pVertexInputState   = &vertexInputInfo,
			 .pInputAssemblyState = &inputAssembly,
			 .pViewportState      = &viewportState,
			 .pRasterizationState = &rasterizer,
			 .pMultisampleState   = &multisampling,
			 .pDepthStencilState  = &depthStencil,
			 .pColorBlendState    = &colorBlending,
			 .pDynamicState       = &dynamicState,
			 .layout              = _pipelineLayout,
			 .renderPass          = nullptr},
			{.colorAttachmentCount = 1, .pColorAttachmentFormats = &_swapChainSurfaceFormat.format, .depthAttachmentFormat = depthFormat}};

		_graphicsPipeline = vk::raii::Pipeline(_device, nullptr, pipelineCreateInfoChain.get<vk::GraphicsPipelineCreateInfo>());
	}




	// endregion

	// region Command Pool
	void Context::createCommandPool() {
		vk::CommandPoolCreateInfo poolInfo{
			.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
			.queueFamilyIndex = _queueFamilyIndices.graphicsFamily.value() };

		_commandPool = vk::raii::CommandPool(_device, poolInfo);

	}

	// endregion

	// region Depth Resources

	vk::Format Context::findSupportedFormat(const std::vector<vk::Format>& candidates, vk::ImageTiling tiling, vk::FormatFeatureFlags features) const {
		for (const auto format : candidates) {
			vk::FormatProperties props = _physicalDevice.getFormatProperties(format);

			if (tiling == vk::ImageTiling::eLinear && (props.linearTilingFeatures & features) == features) {
				return format;
			}
			if (tiling == vk::ImageTiling::eOptimal && (props.optimalTilingFeatures & features) == features) {
				return format;
			}
		}
		throw std::runtime_error("failed to find supported format!");
	}

	vk::Format Context::findDepthFormat() const {
		return findSupportedFormat(
			 {vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint},
				 vk::ImageTiling::eOptimal,
				 vk::FormatFeatureFlagBits::eDepthStencilAttachment
			 );
	}

	bool Context::hasStencilComponent(vk::Format format) {
		return format == vk::Format::eD32SfloatS8Uint || format == vk::Format::eD24UnormS8Uint;
	}

	void Context::createDepthResources() {
		vk::Format depthFormat = findDepthFormat();

		createImage(_swapChainExtent.width, _swapChainExtent.height, depthFormat, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eDepthStencilAttachment, vk::MemoryPropertyFlagBits::eDeviceLocal, _depthImage, _depthImageMemory);
		_depthImageView = createImageView(_depthImage, depthFormat, vk::ImageAspectFlagBits::eDepth);
	}
	// endregion

	// region Buffer Creation and Copy

	void Context::createBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties, vk::raii::Buffer& buffer, vk::raii::DeviceMemory& bufferMemory) {
		vk::BufferCreateInfo bufferInfo{ .size = size, .usage = usage, .sharingMode = vk::SharingMode::eExclusive };
		buffer = vk::raii::Buffer(_device, bufferInfo);
		vk::MemoryRequirements memRequirements = buffer.getMemoryRequirements();
		vk::MemoryAllocateInfo allocInfo{ .allocationSize = memRequirements.size, .memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties) };
		bufferMemory = vk::raii::DeviceMemory(_device, allocInfo);
		buffer.bindMemory(*bufferMemory, 0);
	}

	void Context::copyBuffer(vk::raii::Buffer &srcBuffer, vk::raii::Buffer &dstBuffer, vk::DeviceSize size) {
		vk::raii::CommandBuffer commandCopyBuffer = beginSingleTimeCommands();
		commandCopyBuffer.copyBuffer(srcBuffer, dstBuffer, vk::BufferCopy(0, 0, size));
		endSingleTimeCommands(commandCopyBuffer);
	}
	// endregion

	// region Texture Image
	void Context::createTextureImage() {
		auto image = Utils::readImage("../../textures/texture.jpg");
		vk::DeviceSize imageSize = image.width * image.height * sizeof(image.pixels[0]);

		vk::raii::Buffer       stagingBuffer({});
		vk::raii::DeviceMemory stagingBufferMemory({});
		createBuffer(imageSize, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, stagingBuffer, stagingBufferMemory);

		void *data = stagingBufferMemory.mapMemory(0, imageSize);
		memcpy(data, image.pixels.data(), imageSize);
		stagingBufferMemory.unmapMemory();

		createImage(image.width,
					image.height,
					vk::Format::eR8G8B8A8Srgb,
					vk::ImageTiling::eOptimal,
					vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
					vk::MemoryPropertyFlagBits::eDeviceLocal,
					_textureImage,
					_textureImageMemory);

		transitionImageLayout(_textureImage, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);
		copyBufferToImage(stagingBuffer, _textureImage, static_cast<uint32_t>(image.width), static_cast<uint32_t>(image.height));
		transitionImageLayout(_textureImage, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);
	}

	vk::raii::ImageView Context::createImageView(vk::raii::Image &image, vk::Format format, vk::ImageAspectFlags aspectFlags)
	{
		vk::ImageViewCreateInfo viewInfo{
			.image            = image,
			.viewType         = vk::ImageViewType::e2D,
			.format           = format,
			.subresourceRange = {aspectFlags, 0, 1, 0, 1}};
		return vk::raii::ImageView(_device, viewInfo);
	}

	void Context::createTextureImageView() {
		_textureImageView = createImageView(_textureImage, vk::Format::eR8G8B8A8Srgb, vk::ImageAspectFlagBits::eColor);
	}

	void Context::createTextureSampler() {
		vk::PhysicalDeviceProperties properties = _physicalDevice.getProperties();
		vk::SamplerCreateInfo        samplerInfo{
			.magFilter        = vk::Filter::eLinear,
			.minFilter        = vk::Filter::eLinear,
			.mipmapMode       = vk::SamplerMipmapMode::eLinear,
			.addressModeU     = vk::SamplerAddressMode::eRepeat,
			.addressModeV     = vk::SamplerAddressMode::eRepeat,
			.addressModeW     = vk::SamplerAddressMode::eRepeat,
			.mipLodBias       = 0.0f,
			.anisotropyEnable = vk::True,
			.maxAnisotropy    = properties.limits.maxSamplerAnisotropy,
			.compareEnable    = vk::False,
			.compareOp        = vk::CompareOp::eAlways};

		_textureSampler = vk::raii::Sampler(_device, samplerInfo);
	}

	void Context::createImage(uint32_t width, uint32_t height, vk::Format format, vk::ImageTiling tiling, vk::ImageUsageFlags usage, vk::MemoryPropertyFlags properties, vk::raii::Image &image, vk::raii::DeviceMemory &imageMemory)
	{
		vk::ImageCreateInfo imageInfo{.imageType = vk::ImageType::e2D, .format = format, .extent = {width, height, 1}, .mipLevels = 1, .arrayLayers = 1, .samples = vk::SampleCountFlagBits::e1, .tiling = tiling, .usage = usage, .sharingMode = vk::SharingMode::eExclusive};

		image = vk::raii::Image(_device, imageInfo);

		vk::MemoryRequirements memRequirements = image.getMemoryRequirements();
		vk::MemoryAllocateInfo allocInfo{.allocationSize  = memRequirements.size,
										 .memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties)};
		imageMemory = vk::raii::DeviceMemory(_device, allocInfo);
		image.bindMemory(imageMemory, 0);
	}

	void Context::transitionImageLayout(const vk::raii::Image &image, vk::ImageLayout oldLayout, vk::ImageLayout newLayout)
	{
		auto commandBuffer = beginSingleTimeCommands();

		vk::ImageMemoryBarrier barrier{.oldLayout = oldLayout, .newLayout = newLayout, .image = image, .subresourceRange = {vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1}};

		vk::PipelineStageFlags sourceStage;
		vk::PipelineStageFlags destinationStage;

		if (oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eTransferDstOptimal)
		{
			barrier.srcAccessMask = {};
			barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;

			sourceStage      = vk::PipelineStageFlagBits::eTopOfPipe;
			destinationStage = vk::PipelineStageFlagBits::eTransfer;
		}
		else if (oldLayout == vk::ImageLayout::eTransferDstOptimal && newLayout == vk::ImageLayout::eShaderReadOnlyOptimal)
		{
			barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
			barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

			sourceStage      = vk::PipelineStageFlagBits::eTransfer;
			destinationStage = vk::PipelineStageFlagBits::eFragmentShader;
		}
		else
		{
			throw std::invalid_argument("unsupported layout transition!");
		}
		commandBuffer.pipelineBarrier(sourceStage, destinationStage, {}, {}, nullptr, barrier);
		endSingleTimeCommands(commandBuffer);
	}

	void Context::copyBufferToImage(const vk::raii::Buffer &buffer, vk::raii::Image &image, uint32_t width, uint32_t height)
	{
		vk::raii::CommandBuffer commandBuffer = beginSingleTimeCommands();
		vk::BufferImageCopy                      region{.bufferOffset = 0, .bufferRowLength = 0, .bufferImageHeight = 0, .imageSubresource = {vk::ImageAspectFlagBits::eColor, 0, 0, 1}, .imageOffset = {0, 0, 0}, .imageExtent = {width, height, 1}};
		commandBuffer.copyBufferToImage(buffer, image, vk::ImageLayout::eTransferDstOptimal, {region});
		endSingleTimeCommands(commandBuffer);
	}

	// endregion

	// region Load Model
	void Context::loadModel() {

	}
	// endregion

	// region Vertex Index Buffers
	void Context::createVertexBuffer() {
		vk::DeviceSize         bufferSize = sizeof(vertices[0]) * vertices.size();
		vk::raii::Buffer       stagingBuffer({});
		vk::raii::DeviceMemory stagingBufferMemory({});
		createBuffer(bufferSize, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, stagingBuffer, stagingBufferMemory);

		void *dataStaging = stagingBufferMemory.mapMemory(0, bufferSize);
		memcpy(dataStaging, vertices.data(), bufferSize);
		stagingBufferMemory.unmapMemory();

		createBuffer(bufferSize, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer, vk::MemoryPropertyFlagBits::eDeviceLocal, _vertexBuffer, _vertexBufferMemory);
		copyBuffer(stagingBuffer, _vertexBuffer, bufferSize);
	}

	void Context::createIndexBuffer() {
		vk::DeviceSize bufferSize = sizeof(vertexIndices[0]) * vertexIndices.size();
		vk::raii::Buffer stagingBuffer({});
		vk::raii::DeviceMemory stagingBufferMemory({});
		createBuffer(bufferSize, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, stagingBuffer, stagingBufferMemory);

		void* data = stagingBufferMemory.mapMemory(0, bufferSize);
		memcpy(data, vertexIndices.data(), (size_t) bufferSize);
		stagingBufferMemory.unmapMemory();

		createBuffer(bufferSize, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer, vk::MemoryPropertyFlagBits::eDeviceLocal, _indexBuffer, _indexBufferMemory);
		copyBuffer(stagingBuffer, _indexBuffer, bufferSize);
	}

	// endregion

	// region Uniform Buffers and Descriptor Sets
	void Context::createUniformBuffers() {
		_uniformBuffers.clear();
		_uniformBuffersMemory.clear();
		_uniformBuffersMapped.clear();

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			vk::DeviceSize bufferSize = sizeof(UniformBufferObject);
			vk::raii::Buffer buffer({});
			vk::raii::DeviceMemory bufferMem({});
			createBuffer(bufferSize, vk::BufferUsageFlagBits::eUniformBuffer, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, buffer, bufferMem);
			_uniformBuffers.emplace_back(std::move(buffer));
			_uniformBuffersMemory.emplace_back(std::move(bufferMem));
			_uniformBuffersMapped.emplace_back( _uniformBuffersMemory[i].mapMemory(0, bufferSize));
		}
	}

	void Context::createDescriptorPool() {
		std::array poolSize {
			vk::DescriptorPoolSize( vk::DescriptorType::eUniformBuffer, MAX_FRAMES_IN_FLIGHT),
			vk::DescriptorPoolSize(  vk::DescriptorType::eCombinedImageSampler, MAX_FRAMES_IN_FLIGHT)
		};
		vk::DescriptorPoolCreateInfo poolInfo{.flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet, .maxSets = MAX_FRAMES_IN_FLIGHT, .poolSizeCount = poolSize.size(), .pPoolSizes = poolSize.data()};
		_descriptorPool = vk::raii::DescriptorPool(_device, poolInfo);
	}

	void Context::createDescriptorSets() {
		std::vector<vk::DescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, *_descriptorSetLayout);
		vk::DescriptorSetAllocateInfo allocInfo{
			.descriptorPool = _descriptorPool,
			.descriptorSetCount = static_cast<uint32_t>(layouts.size()),
			.pSetLayouts = layouts.data()
		};

		_descriptorSets = _device.allocateDescriptorSets(allocInfo);

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		{
			vk::DescriptorBufferInfo bufferInfo{ .buffer = _uniformBuffers[i], .offset = 0, .range = sizeof(UniformBufferObject) };
			vk::DescriptorImageInfo imageInfo{ .sampler = _textureSampler, .imageView = _textureImageView, .imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal};

			std::array descriptorWrites{
				vk::WriteDescriptorSet{ .dstSet = _descriptorSets[i], .dstBinding = 0, .dstArrayElement = 0, .descriptorCount = 1,
					.descriptorType = vk::DescriptorType::eUniformBuffer, .pBufferInfo = &bufferInfo },
				vk::WriteDescriptorSet{ .dstSet = _descriptorSets[i], .dstBinding = 1, .dstArrayElement = 0, .descriptorCount = 1,
					.descriptorType = vk::DescriptorType::eCombinedImageSampler, .pImageInfo = &imageInfo }
			};
			_device.updateDescriptorSets(descriptorWrites, {});
		}
	}

	void Context::updateUniformBuffer(uint32_t frameIndex) {
		static auto startTime = std::chrono::high_resolution_clock::now();

		auto currentTime = std::chrono::high_resolution_clock::now();
		float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

		UniformBufferObject ubo{};
		ubo.model = rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));

		ubo.view = lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));

		ubo.proj = glm::perspective(glm::radians(45.0f), static_cast<float>(_swapChainExtent.width) / static_cast<float>(_swapChainExtent.height), 0.1f, 10.0f);

		ubo.proj[1][1] *= -1;

		memcpy(_uniformBuffersMapped[frameIndex], &ubo, sizeof(ubo));
	}

	// endregion

	uint32_t Context::findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties) {
		const vk::PhysicalDeviceMemoryProperties memProperties = _physicalDevice.getMemoryProperties();

		for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
			if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
				return i;

		throw std::runtime_error("failed to find suitable memory type!");
	}


	// region Command Buffers

	void Context::createCommandBuffers() {
		vk::CommandBufferAllocateInfo allocInfo{
			.commandPool = _commandPool,
			.level = vk::CommandBufferLevel::ePrimary,
			.commandBufferCount = MAX_FRAMES_IN_FLIGHT };

		_commandBuffers = vk::raii::CommandBuffers(_device, allocInfo);
	}

	void Context::recordCommandBuffer(uint32_t imageIndex) {

		const auto & _commandBuffer = _commandBuffers[_frameIndex];

		_commandBuffer.begin({});

		// Before starting rendering, transition the swapchain image to COLOR_ATTACHMENT_OPTIMAL
		recordTransitionImageLayout(
			_swapChainImages[imageIndex],
			vk::ImageLayout::eUndefined,
			vk::ImageLayout::eColorAttachmentOptimal,
			{},                                                         // srcAccessMask (no need to wait for previous operations)
			vk::AccessFlagBits2::eColorAttachmentWrite,                 // dstAccessMask
			vk::PipelineStageFlagBits2::eColorAttachmentOutput,         // srcStage
			vk::PipelineStageFlagBits2::eColorAttachmentOutput,         // dstStage
			vk::ImageAspectFlagBits::eColor           		           // image_aspect_flags
		);
		// Transition for the depth image
		recordTransitionImageLayout(
			*_depthImage,
			vk::ImageLayout::eUndefined,
			vk::ImageLayout::eDepthAttachmentOptimal,
			vk::AccessFlagBits2::eDepthStencilAttachmentWrite,
			vk::AccessFlagBits2::eDepthStencilAttachmentWrite,
			vk::PipelineStageFlagBits2::eEarlyFragmentTests | vk::PipelineStageFlagBits2::eLateFragmentTests,
			vk::PipelineStageFlagBits2::eEarlyFragmentTests | vk::PipelineStageFlagBits2::eLateFragmentTests,
			vk::ImageAspectFlagBits::eDepth);

		vk::ClearValue clearColor = vk::ClearColorValue(0.0f, 0.0f, 0.0f, 1.0f);
		vk::ClearValue clearDepth = vk::ClearDepthStencilValue(1.0f, 0);
		vk::RenderingAttachmentInfo attachmentInfo = {
			.imageView = _swapChainImageViews[imageIndex],
			.imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
			.loadOp = vk::AttachmentLoadOp::eClear,
			.storeOp = vk::AttachmentStoreOp::eStore,
			.clearValue = clearColor
		};
		vk::RenderingAttachmentInfo depthAttachmentInfo = {
			.imageView   = _depthImageView,
			.imageLayout = vk::ImageLayout::eDepthAttachmentOptimal,
			.loadOp      = vk::AttachmentLoadOp::eClear,
			.storeOp     = vk::AttachmentStoreOp::eDontCare,
			.clearValue  = clearDepth
		};

		vk::RenderingInfo renderingInfo = {
			.renderArea = { .offset = { 0, 0 }, .extent = _swapChainExtent },
			.layerCount = 1,
			.colorAttachmentCount = 1,
			.pColorAttachments = &attachmentInfo,
			.pDepthAttachment = &depthAttachmentInfo
		};

		_commandBuffer.beginRendering(renderingInfo);

		_commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, _graphicsPipeline);
		_commandBuffer.setViewport(0, vk::Viewport(0.0f, 0.0f, static_cast<float>(_swapChainExtent.width), static_cast<float>(_swapChainExtent.height), 0.0f, 1.0f));
		_commandBuffer.setScissor(0, vk::Rect2D(vk::Offset2D(0, 0), _swapChainExtent));
		_commandBuffer.bindVertexBuffers(0, *_vertexBuffer, {0});
		_commandBuffer.bindIndexBuffer( *_indexBuffer, 0, vk::IndexType::eUint16 );
		_commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, _pipelineLayout, 0, *_descriptorSets[_frameIndex], nullptr);
		_commandBuffer.drawIndexed(vertexIndices.size(), 1, 0, 0, 0);

		_commandBuffer.endRendering();

		// After rendering, transition the swapchain image to PRESENT_SRC
		recordTransitionImageLayout(
			_swapChainImages[imageIndex],
			vk::ImageLayout::eColorAttachmentOptimal,
			vk::ImageLayout::ePresentSrcKHR,
			vk::AccessFlagBits2::eColorAttachmentWrite,             // srcAccessMask
			{},                                                     // dstAccessMask
			vk::PipelineStageFlagBits2::eColorAttachmentOutput,     // srcStage
			vk::PipelineStageFlagBits2::eBottomOfPipe,         // dstStage
			vk::ImageAspectFlagBits::eColor           		           // image_aspect_flags
		);

		_commandBuffer.end();
	}

	vk::raii::CommandBuffer Context::beginSingleTimeCommands() {
		vk::CommandBufferAllocateInfo allocInfo{ .commandPool = _commandPool, .level = vk::CommandBufferLevel::ePrimary, .commandBufferCount = 1 };
		vk::raii::CommandBuffer commandBuffer = std::move(_device.allocateCommandBuffers(allocInfo).front());

		vk::CommandBufferBeginInfo beginInfo{ .flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit };
		commandBuffer.begin(beginInfo);

		return commandBuffer;
	}

	void Context::endSingleTimeCommands(vk::raii::CommandBuffer& commandBuffer) {
		commandBuffer.end();

		vk::SubmitInfo submitInfo{ .commandBufferCount = 1, .pCommandBuffers = &*commandBuffer };
		_graphicsQueue.submit(submitInfo, nullptr);
		_graphicsQueue.waitIdle();
	}

	void Context::recordTransitionImageLayout(
		vk::Image image,
		vk::ImageLayout oldLayout,
		vk::ImageLayout newLayout,
		vk::AccessFlags2 srcAccessMask,
		vk::AccessFlags2 dstAccessMask,
		vk::PipelineStageFlags2 srcStageMask,
		vk::PipelineStageFlags2 dstStageMask,
		vk::ImageAspectFlags    image_aspect_flags
	) const {
		vk::ImageMemoryBarrier2 barrier = {
			.srcStageMask = srcStageMask,
			.srcAccessMask = srcAccessMask,
			.dstStageMask = dstStageMask,
			.dstAccessMask = dstAccessMask,
			.oldLayout = oldLayout,
			.newLayout = newLayout,
			.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.image = image,
			.subresourceRange = {
				.aspectMask = image_aspect_flags,
				.baseMipLevel = 0,
				.levelCount = 1,
				.baseArrayLayer = 0,
				.layerCount = 1
			}
		};
		vk::DependencyInfo dependencyInfo = {
			.dependencyFlags = {},
			.imageMemoryBarrierCount = 1,
			.pImageMemoryBarriers = &barrier
		};
		_commandBuffers[_frameIndex].pipelineBarrier2(dependencyInfo);
	}


	// endregion

	// region Sync Objects
	void Context::createSyncObjects() {
		assert(_presentCompleteSemaphores.empty() && _renderFinishedSemaphores.empty() && _inFlightFences.empty());

		for (size_t i = 0; i < _swapChainImages.size(); i++)
			_renderFinishedSemaphores.emplace_back(_device, vk::SemaphoreCreateInfo());

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
			_presentCompleteSemaphores.emplace_back(_device, vk::SemaphoreCreateInfo());

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
			_inFlightFences.emplace_back(_device, vk::FenceCreateInfo{.flags = vk::FenceCreateFlagBits::eSignaled});
	}


	// endregion


}
