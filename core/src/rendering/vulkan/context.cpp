//
// Created by eharquin on 12/7/25.
//


#include <core/vulkan/context.hpp>

#include <algorithm>
#include <stdexcept>
#include <iostream>
#include <string>
#include <map>

#define GLFW_INCLUDE_VULKAN
#include <numeric>
#include <GLFW/glfw3.h>

#include <core/utils/file_utils.hpp>

namespace Core::Vulkan {
	void Context::create(const Window & window) {
		createInstance();
		setupDebugMessenger();
		createSurface(window);
		pickPhysicalDevice();
		createLogicalDevice();
		createSwapChain(window);
		createImageViews();
		createGraphicsPipeline();
		createCommandPool();
		createCommandBuffers();
		createSyncObjects();
	}

	void Context::drawFrame(Window & window) {

		// Note: inFlightFences, presentCompleteSemaphores, and commandBuffers are indexed by frameIndex,
		//       while renderFinishedSemaphores is indexed by imageIndex
		while (vk::Result::eTimeout == _device.waitForFences(*_inFlightFences[frameIndex], vk::True, UINT64_MAX));

		auto [result, imageIndex] = _swapChain.acquireNextImage( UINT64_MAX, *_presentCompleteSemaphores[frameIndex], nullptr );

		if (result == vk::Result::eErrorOutOfDateKHR) {
			recreateSwapChain(window);
			std::cout << "Swap chain recreated due to eErrorOutOfDateKHR during acquireNextImage.\n";
			return;
		}
		if (result != vk::Result::eSuccess && result != vk::Result::eSuboptimalKHR) {
			throw std::runtime_error("failed to acquire swap chain image!");
		}

		_device.resetFences(*_inFlightFences[frameIndex]);

		_commandBuffers[frameIndex].reset();
		recordCommandBuffer(imageIndex);


		vk::PipelineStageFlags waitDestinationStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput);

		const vk::SubmitInfo submitInfo{
			.waitSemaphoreCount = 1,
			.pWaitSemaphores = &*_presentCompleteSemaphores[frameIndex],
			.pWaitDstStageMask = &waitDestinationStageMask,
			.commandBufferCount = 1,
			.pCommandBuffers = &*_commandBuffers[frameIndex],
			.signalSemaphoreCount = 1,
			.pSignalSemaphores = &*_renderFinishedSemaphores[imageIndex]
		};

		_graphicsQueue.submit(submitInfo, *_inFlightFences[frameIndex]);

		try
		{
			const vk::PresentInfoKHR presentInfoKHR{.waitSemaphoreCount = 1,
													.pWaitSemaphores    = &*_renderFinishedSemaphores[imageIndex],
													.swapchainCount     = 1,
													.pSwapchains        = &*_swapChain,
													.pImageIndices      = &imageIndex};
			result = _graphicsQueue.presentKHR(presentInfoKHR);
			if (result == vk::Result::eSuboptimalKHR || window.framebufferResized())
			{
				std::cout << "Swap chain recreated due to eSuboptimalKHR or window resize during presentKHR.\n";
				window.resetFramebufferResized();
				recreateSwapChain(window);
			}
			else if (result != vk::Result::eSuccess)
				throw std::runtime_error("failed to present swap chain image!");
		}
		catch (const vk::SystemError &e)
		{
			if (e.code().value() == static_cast<int>(vk::Result::eErrorOutOfDateKHR))
			{
				std::cout << "Swap chain recreated due to eErrorOutOfDateKHR exception during presentKHR.\n";
				recreateSwapChain(window);
				return;
			}
			else
			{
				throw;
			}
		}

		frameIndex = (frameIndex + 1) % MAX_FRAMES_IN_FLIGHT;
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
													features.template get<vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>().extendedDynamicState;

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
			{},                                    // vk::PhysicalDeviceFeatures2
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
	void Context::createSwapChain(const Window & window) {
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
		auto chooseSwapExtent = [&window](const vk::SurfaceCapabilitiesKHR& capabilities) {
			if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) return capabilities.currentExtent;

			int width, height;
			glfwGetFramebufferSize(window.glfwHandle(), &width, &height);

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

	void Context::recreateSwapChain(const Window & window) {
		while (window.isMinimized())
			window.pollEvents();

		_device.waitIdle();

		cleanupSwapChain();

		createSwapChain(window);
		createImageViews();
	}

	// endregion

	// region Image Views
	void Context::createImageViews() {
		_swapChainImageViews.clear();

		// vk::ImageViewCreateInfo imageViewCreateInfo( {}, {}, vk::ImageViewType::e2D, swapChainImageFormat, {}, { vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 } );
		vk::ImageViewCreateInfo imageViewCreateInfo{
			.viewType = vk::ImageViewType::e2D,
			.format = _swapChainSurfaceFormat.format,
			.subresourceRange = {
					.aspectMask = vk::ImageAspectFlagBits::eColor,
		  			.baseMipLevel = 0,
		  			.levelCount = 1,
		  			.baseArrayLayer = 0,
		  			.layerCount = 1 }
		};

		std::cout << "[ASTRO CORE] [VULKAN] [SWAPCHAIN] image view infos:\n"
		  << "  format: " << to_string(imageViewCreateInfo.format) << "\n"
		  << "  viewType: " << to_string(imageViewCreateInfo.viewType) << "\n"
		  << "  subresourceRange:\n"
		  << "    aspectMask: " << to_string(imageViewCreateInfo.subresourceRange.aspectMask) << "\n"
		  << "    baseMipLevel: " << imageViewCreateInfo.subresourceRange.baseMipLevel << "\n"
		  << "    levelCount: " << imageViewCreateInfo.subresourceRange.levelCount << "\n"
		  << "    baseArrayLayer: " << imageViewCreateInfo.subresourceRange.baseArrayLayer << "\n"
		  << "    layerCount: " << imageViewCreateInfo.subresourceRange.layerCount << std::endl;

		for (const auto image : _swapChainImages) {
			imageViewCreateInfo.image = image;
			_swapChainImageViews.emplace_back( _device, imageViewCreateInfo );
		}

		std::cout << "[ASTRO CORE] [VULKAN] [SWAPCHAIN] created " <<
			_swapChainImageViews.size() << " image views" << std::endl;
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
		const vk::raii::ShaderModule shaderModule = createShaderModule(readFile("../../shaders/slang.spv"));

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


		vk::PipelineVertexInputStateCreateInfo   vertexInputInfo;
		vk::PipelineInputAssemblyStateCreateInfo inputAssembly{.topology = vk::PrimitiveTopology::eTriangleList};
		vk::PipelineViewportStateCreateInfo      viewportState{.viewportCount = 1, .scissorCount = 1};

		vk::PipelineRasterizationStateCreateInfo rasterizer{
			.depthClampEnable = vk::False,
			.rasterizerDiscardEnable = vk::False,
			.polygonMode = vk::PolygonMode::eFill,
			.cullMode = vk::CullModeFlagBits::eBack,
			.frontFace = vk::FrontFace::eClockwise,
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

		vk::PipelineLayoutCreateInfo pipelineLayoutInfo{.setLayoutCount = 0, .pushConstantRangeCount = 0};

		_pipelineLayout = vk::raii::PipelineLayout(_device, pipelineLayoutInfo);

		vk::StructureChain<vk::GraphicsPipelineCreateInfo, vk::PipelineRenderingCreateInfo> pipelineCreateInfoChain = {
			{.stageCount          = 2,
			 .pStages             = shaderStages,
			 .pVertexInputState   = &vertexInputInfo,
			 .pInputAssemblyState = &inputAssembly,
			 .pViewportState      = &viewportState,
			 .pRasterizationState = &rasterizer,
			 .pMultisampleState   = &multisampling,
			 .pColorBlendState    = &colorBlending,
			 .pDynamicState       = &dynamicState,
			 .layout              = _pipelineLayout,
			 .renderPass          = nullptr},
			{.colorAttachmentCount = 1, .pColorAttachmentFormats = &_swapChainSurfaceFormat.format}};

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

	void Context::createCommandBuffers() {
		vk::CommandBufferAllocateInfo allocInfo{
			.commandPool = _commandPool,
			.level = vk::CommandBufferLevel::ePrimary,
			.commandBufferCount = MAX_FRAMES_IN_FLIGHT };

		_commandBuffers = vk::raii::CommandBuffers(_device, allocInfo);
	}

	void Context::recordCommandBuffer(uint32_t imageIndex) {

		const auto & _commandBuffer = _commandBuffers[frameIndex];

		_commandBuffer.begin({});

		// Before starting rendering, transition the swapchain image to COLOR_ATTACHMENT_OPTIMAL
		transitionImageLayout(
			imageIndex,
			vk::ImageLayout::eUndefined,
			vk::ImageLayout::eColorAttachmentOptimal,
			{},                                                         // srcAccessMask (no need to wait for previous operations)
			vk::AccessFlagBits2::eColorAttachmentWrite,                 // dstAccessMask
			vk::PipelineStageFlagBits2::eColorAttachmentOutput,         // srcStage
			vk::PipelineStageFlagBits2::eColorAttachmentOutput          // dstStage
		);

		vk::ClearValue clearColor = vk::ClearColorValue(0.0f, 0.0f, 0.0f, 1.0f);
		vk::RenderingAttachmentInfo attachmentInfo = {
			.imageView = _swapChainImageViews[imageIndex],
			.imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
			.loadOp = vk::AttachmentLoadOp::eClear,
			.storeOp = vk::AttachmentStoreOp::eStore,
			.clearValue = clearColor
		};

		vk::RenderingInfo renderingInfo = {
			.renderArea = { .offset = { 0, 0 }, .extent = _swapChainExtent },
			.layerCount = 1,
			.colorAttachmentCount = 1,
			.pColorAttachments = &attachmentInfo
		};

		_commandBuffer.beginRendering(renderingInfo);

		_commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, _graphicsPipeline);
		_commandBuffer.setViewport(0, vk::Viewport(0.0f, 0.0f, static_cast<float>(_swapChainExtent.width), static_cast<float>(_swapChainExtent.height), 0.0f, 1.0f));
		_commandBuffer.setScissor(0, vk::Rect2D(vk::Offset2D(0, 0), _swapChainExtent));
		_commandBuffer.draw(3, 1, 0, 0);

		_commandBuffer.endRendering();

		// After rendering, transition the swapchain image to PRESENT_SRC
		transitionImageLayout(
			imageIndex,
			vk::ImageLayout::eColorAttachmentOptimal,
			vk::ImageLayout::ePresentSrcKHR,
			vk::AccessFlagBits2::eColorAttachmentWrite,             // srcAccessMask
			{},                                                     // dstAccessMask
			vk::PipelineStageFlagBits2::eColorAttachmentOutput,     // srcStage
			vk::PipelineStageFlagBits2::eBottomOfPipe               // dstStage
		);

		_commandBuffer.end();
	}

	void Context::transitionImageLayout(
		uint32_t imageIndex,
		vk::ImageLayout oldLayout,
		vk::ImageLayout newLayout,
		vk::AccessFlags2 srcAccessMask,
		vk::AccessFlags2 dstAccessMask,
		vk::PipelineStageFlags2 srcStageMask,
		vk::PipelineStageFlags2 dstStageMask
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
			.image = _swapChainImages[imageIndex],
			.subresourceRange = {
				.aspectMask = vk::ImageAspectFlagBits::eColor,
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
		_commandBuffers[frameIndex].pipelineBarrier2(dependencyInfo);
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
