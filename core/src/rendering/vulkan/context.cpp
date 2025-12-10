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
#include <GLFW/glfw3.h>

namespace Core::Vulkan {
	void Context::create() {
		createInstance();
		pickPhysicalDevice();
		createLogicalDevice();
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
				std::cerr << "[VULKAN Instance] Required extension not supported: " + std::string(requiredExtension) <<
						std::endl;
			} else {
				std::cout << "[VULKAN Instance] Required extension found: " + std::string(requiredExtension) <<
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
				std::cerr << "[VULKAN Instance] Required layer not supported: " + std::string(requiredLayer) <<
						std::endl;
			} else {
				std::cout << "[VULKAN Instance] Required layer found: " + std::string(requiredLayer) << std::endl;
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
					bool isSuitable = device.getProperties().apiVersion >= VK_API_VERSION_1_3;

					// Check for graphics queue support
					auto queueFamilies = device.getQueueFamilyProperties();
					const auto qfpIter =
							std::ranges::find_if(queueFamilies, [](vk::QueueFamilyProperties const &qfp) {
								return (qfp.queueFlags & vk::QueueFlagBits::eGraphics) != static_cast<vk::QueueFlags>(
									       0);
							});
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
					}
					isSuitable = isSuitable && found;

					if (isSuitable)
						_physicalDevice = device;

					return isSuitable;
				});
		if (devIter == devices.end()) {
			throw std::runtime_error("failed to find a suitable GPU!");
		}
	}


	static uint32_t findQueueFamilies(const vk::raii::PhysicalDevice &physicalDevice) {
		// find the index of the first queue family that supports graphics
		std::vector<vk::QueueFamilyProperties> queueFamilyProperties = physicalDevice.getQueueFamilyProperties();

		// get the first index into queueFamilyProperties which supports graphics
		const auto graphicsQueueFamilyProperty =
				std::find_if(queueFamilyProperties.begin(),
				             queueFamilyProperties.end(),
				             [](vk::QueueFamilyProperties const &qfp) {
					             return qfp.queueFlags & vk::QueueFlagBits::eGraphics;
				             });

		return static_cast<uint32_t>(std::distance(queueFamilyProperties.begin(), graphicsQueueFamilyProperty));
	}

	void Context::createLogicalDevice() {
		std::vector<vk::QueueFamilyProperties> queueFamilyProperties = _physicalDevice.getQueueFamilyProperties();
		const uint32_t graphicsIndex = findQueueFamilies(_physicalDevice);
		float queuePriority = 0.5f;
		vk::DeviceQueueCreateInfo deviceQueueCreateInfo{
			.queueFamilyIndex = graphicsIndex,
			.queueCount = 1,
			.pQueuePriorities = &queuePriority
		};

		// TO COMPLETE FURTHER
		vk::PhysicalDeviceFeatures deviceFeatures;

		// Create a chain of feature structures
		vk::StructureChain<vk::PhysicalDeviceFeatures2, vk::PhysicalDeviceVulkan13Features,
			vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT> featureChain = {
			{}, // vk::PhysicalDeviceFeatures2 (empty for now)
			{.dynamicRendering = true}, // Enable dynamic rendering from Vulkan 1.3
			{.extendedDynamicState = true} // Enable extended dynamic state from the extension
		};

		vk::DeviceCreateInfo deviceCreateInfo{
			.pNext = &featureChain.get<vk::PhysicalDeviceFeatures2>(),
			.queueCreateInfoCount = 1,
			.pQueueCreateInfos = &deviceQueueCreateInfo,
			.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size()),
			.ppEnabledExtensionNames = deviceExtensions.data()
		};

		_device = vk::raii::Device(_physicalDevice, deviceCreateInfo);
		_graphicsQueue = vk::raii::Queue(_device, graphicsIndex, 0);
	}

	// endregion
} // namespace Core::Vulkan
