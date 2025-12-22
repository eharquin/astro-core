//
// Created by eharquin on 12/20/25.
//

#include <cstring>
#include <iostream>
#include <set>
#include <core/rendering/vulkan/Context.hpp>
#include "core/rendering/vulkan/Renderer.hpp"

namespace Core::Rendering::Vulkan {

	void Context::init(const Window &window)
	{
		create(window);
	}

	void Context::shutdown() {
		if (*_device)
			_device.waitIdle();
	}

	std::unique_ptr<IRenderer> Context::createRenderer(Window &window) {
		return std::make_unique<Renderer>(*this, window);
	}

	void Context::create(const Window &window) {
		createInstance();
		if (enableValidationLayers)
			setupDebugMessenger();

		createSurface(window);
		pickPhysicalDevice();
		createLogicalDevice();
		createCommandPool();
	}

	void Context::waitIdle() {
		_device.waitIdle();
	}

	uint32_t Context::findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties) const {
		const vk::PhysicalDeviceMemoryProperties memProperties = _physicalDevice.getMemoryProperties();

		for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
			if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
				return i;

		throw std::runtime_error("failed to find suitable memory type!");
	}

	vk::raii::ImageView Context::createImageView(vk::raii::Image& image, vk::Format format, vk::ImageAspectFlags aspectFlags) const {
		vk::ImageViewCreateInfo viewInfo{
			.image = image,
			.viewType = vk::ImageViewType::e2D,
			.format = format,
			.subresourceRange = {aspectFlags, 0, 1, 0, 1}
		};
		return vk::raii::ImageView(_device, viewInfo);
	}
	vk::raii::ImageView Context::createImageView(vk::Image& image, vk::Format format, vk::ImageAspectFlags aspectFlags) const {
		vk::ImageViewCreateInfo viewInfo{
			.image = image,
			.viewType = vk::ImageViewType::e2D,
			.format = format,
			.subresourceRange = {aspectFlags, 0, 1, 0, 1}
		};
		return vk::raii::ImageView(_device, viewInfo);
	}

	void Context::createImage(uint32_t width, uint32_t height, vk::Format format, vk::ImageTiling tiling,
		vk::ImageUsageFlags usage, vk::MemoryPropertyFlags properties, vk::raii::Image &image,
		vk::raii::DeviceMemory &imageMemory)
	{
		vk::ImageCreateInfo imageInfo{.imageType = vk::ImageType::e2D, .format = format, .extent = {width, height, 1}, .mipLevels = 1, .arrayLayers = 1, .samples = vk::SampleCountFlagBits::e1, .tiling = tiling, .usage = usage, .sharingMode = vk::SharingMode::eExclusive};

		image = vk::raii::Image(_device, imageInfo);

		vk::MemoryRequirements memRequirements = image.getMemoryRequirements();
		vk::MemoryAllocateInfo allocInfo{.allocationSize  = memRequirements.size,
										 .memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties)};
		imageMemory = vk::raii::DeviceMemory(_device, allocInfo);
		image.bindMemory(imageMemory, 0);
	}

	void Context::createBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties, vk::raii::Buffer& buffer, vk::raii::DeviceMemory& bufferMemory) const {
		vk::BufferCreateInfo bufferInfo{ .size = size, .usage = usage, .sharingMode = vk::SharingMode::eExclusive };
		buffer = vk::raii::Buffer(_device, bufferInfo);
		vk::MemoryRequirements memRequirements = buffer.getMemoryRequirements();
		vk::MemoryAllocateInfo allocInfo{ .allocationSize = memRequirements.size, .memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties) };
		bufferMemory = vk::raii::DeviceMemory(_device, allocInfo);
		buffer.bindMemory(*bufferMemory, 0);
	}

	void Context::copyBuffer(vk::raii::Buffer &srcBuffer, vk::raii::Buffer &dstBuffer, vk::DeviceSize size) const {
		vk::raii::CommandBuffer commandCopyBuffer = beginSingleTimeCommands();
		commandCopyBuffer.copyBuffer(srcBuffer, dstBuffer, vk::BufferCopy(0, 0, size));
		endSingleTimeCommands(commandCopyBuffer);
	}

	void Context::createStagingBuffer(const void* data, vk::DeviceSize size, vk::raii::Buffer& stagingBuffer, vk::raii::DeviceMemory& stagingMemory) const {
		createBuffer(
			size,
			vk::BufferUsageFlagBits::eTransferSrc,
			vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
			stagingBuffer,
			stagingMemory
		);

		void* mapped = stagingMemory.mapMemory(0, size);
		memcpy(mapped, data, static_cast<size_t>(size));
		stagingMemory.unmapMemory();
	}

	vk::raii::CommandBuffer Context::beginSingleTimeCommands() const {
		vk::CommandBufferAllocateInfo allocInfo{ .commandPool = _commandPool, .level = vk::CommandBufferLevel::ePrimary, .commandBufferCount = 1 };
		vk::raii::CommandBuffer commandBuffer = std::move(_device.allocateCommandBuffers(allocInfo).front());
		vk::CommandBufferBeginInfo beginInfo{ .flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit };
		commandBuffer.begin(beginInfo);
		return commandBuffer;
	}

	void Context::endSingleTimeCommands(vk::raii::CommandBuffer& commandBuffer) const {
		commandBuffer.end();
		vk::SubmitInfo submitInfo{ .commandBufferCount = 1, .pCommandBuffers = &*commandBuffer };
		_graphicsQueue.submit(submitInfo, nullptr);
		_graphicsQueue.waitIdle();
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

	void Context::createTextureImageFromData(const void* pixels,uint32_t width,uint32_t height,vk::raii::Image& outImage,vk::raii::DeviceMemory& outMemory)
	{
		vk::DeviceSize imageSize = width * height * 4; // assuming RGBA

		// Staging buffer
		vk::raii::Buffer stagingBuffer(nullptr);
		vk::raii::DeviceMemory stagingMemory(nullptr);
		createStagingBuffer(pixels, imageSize, stagingBuffer, stagingMemory);

		// Device-local image
		createImage(
			width,
			height,
			vk::Format::eR8G8B8A8Srgb,
			vk::ImageTiling::eOptimal,
			vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
			vk::MemoryPropertyFlagBits::eDeviceLocal,
			outImage,
			outMemory
		);

		// Copy from staging buffer
		transitionImageLayout(outImage, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);
		copyBufferToImage(stagingBuffer, outImage, width, height);
		transitionImageLayout(outImage, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);
	}

	vk::raii::Sampler Context::createTextureSampler() {
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

		return vk::raii::Sampler(_device, samplerInfo);
	}


	// region Instance Creation
	void Context::createInstance() {
		vk::ApplicationInfo appInfo{
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

	QueueFamilyIndices Context::findQueueFamilies(const vk::raii::PhysicalDevice &physicalDevice, const vk::raii::SurfaceKHR &surface) {

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

		std::set<uint32_t> uniqueFamilies = {
			_queueFamilyIndices.graphicsFamily.value(),
			_queueFamilyIndices.presentFamily.value()
		};

		std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
		float queuePriority = 1.0f;
		for (uint32_t family : uniqueFamilies) {
			queueCreateInfos.push_back({
				.queueFamilyIndex = family,
				.queueCount = 1,
				.pQueuePriorities = &queuePriority
			});
		}

		// TO COMPLETE FURTHER
		vk::PhysicalDeviceFeatures deviceFeatures;

		// Create a chain of feature structures
		// Feature chain (Vulkan 1.1, 1.3 + extended dynamic state)
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
			.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size()),
			.pQueueCreateInfos = queueCreateInfos.data(),
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

	// region Command Pool
	void Context::createCommandPool() {
		vk::CommandPoolCreateInfo poolInfo{
			.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
			.queueFamilyIndex = _queueFamilyIndices.graphicsFamily.value() };

		_commandPool = vk::raii::CommandPool(_device, poolInfo);
	}

	// endregion
}
