#include <core/vulkan/instance.hpp>
#include <core/vulkan/config.hpp>

#include <algorithm>
#include <stdexcept>
#include <iostream>
#include <string>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace Core::Vulkan {
	bool Instance::checkExtensionSupport(const std::vector<const char *> &requiredExtensions) const {
		bool foundAll = true;
		// Check if the required GLFW extensions are supported by the Vulkan implementation.
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


	bool Instance::checkLayerSupport(const std::vector<const char *> &requiredLayers) const {
		bool foundAll = true;
		// Check if the required GLFW extensions are supported by the Vulkan implementation.
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

	vk::Bool32 Instance::debugCallback(vk::DebugUtilsMessageSeverityFlagBitsEXT severity,
	                                   vk::DebugUtilsMessageTypeFlagsEXT type,
	                                   const vk::DebugUtilsMessengerCallbackDataEXT *pCallbackData, void *) {
		std::cerr << "[VALIDATION LAYER]  type: " << to_string(type) << " msg: " << pCallbackData->pMessage <<
				std::endl;
		return vk::False;
	}

	void Instance::setupDebugMessenger() {
		constexpr vk::DebugUtilsMessageSeverityFlagsEXT severityFlags(
			vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose | vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
			vk::DebugUtilsMessageSeverityFlagBitsEXT::eError);

		constexpr vk::DebugUtilsMessageTypeFlagsEXT messageTypeFlags(
			vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance |
			vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation);

		constexpr vk::DebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCreateInfoEXT{
			.messageSeverity = severityFlags,
			.messageType = messageTypeFlags,
			.pfnUserCallback = &debugCallback
		};

		_debugMessenger = _instance.createDebugUtilsMessengerEXT(debugUtilsMessengerCreateInfoEXT);
	}

	void Instance::create() {
		// if (enableValidationLayers && !checkValidationLayerSupport())
		// 	throw std::runtime_error("validation layers requested, but not available!");

		constexpr vk::ApplicationInfo appInfo{
			.pApplicationName = "Hello Triangle",
			.applicationVersion = VK_MAKE_VERSION(1, 0, 0),
			.pEngineName = "No Engine",
			.engineVersion = VK_MAKE_VERSION(1, 0, 0),
			.apiVersion = vk::ApiVersion14
		};

		auto requiredExtensions = getRequiredExtensions();
		if (!checkExtensionSupport(requiredExtensions))
			throw std::runtime_error("Required extensions not supported");

		std::vector<const char *> requiredLayers;
		if (enableValidationLayers)
			requiredLayers.assign(validationLayers.begin(), validationLayers.end());

		if (!checkLayerSupport(requiredLayers))
			throw std::runtime_error("Required layers not supported");

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

	std::vector<const char *> Instance::getRequiredExtensions() {
		// Get GLFW extensions needed
		uint32_t glfwExtensionCount = 0;
		auto glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

		std::vector<const char *> extensions{
			glfwExtensions,
			glfwExtensions + glfwExtensionCount
		};

		// Add EXT_DEBUG_UTILS extensions IF Validation Layers enable
		if (enableValidationLayers)
			extensions.push_back(vk::EXTDebugUtilsExtensionName);
		return extensions;
	}
} // namespace Core::Vulkan
