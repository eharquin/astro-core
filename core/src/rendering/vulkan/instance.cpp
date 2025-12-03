#include <core/vulkan/instance.hpp>

#include <algorithm>
#include <stdexcept>
#include <iostream>
#include <string>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace Core::Vulkan
{
bool Instance::checkGLFWExtensionSupport(const std::vector<const char *> &requiredExtensions) const
{
	bool foundAll = true;
	// Check if the required GLFW extensions are supported by the Vulkan implementation.
	auto extensionProperties = _context.enumerateInstanceExtensionProperties();
	for (auto requiredExtension : requiredExtensions)
	{
		if (std::ranges::none_of(extensionProperties,
		                         [requiredExtension](auto const &ext) {
			                         return std::string_view(ext.extensionName) == requiredExtension;
		                         }))
		{
			foundAll = false;
			std::cerr << "[VULKAN Instance] Required extension not supported: " + std::string(requiredExtension) << std::endl;
		}
		else
		{
			std::cout << "[VULKAN Instance] Required extension found: " + std::string(requiredExtension) << std::endl;
		}
	}

	return foundAll;
}

void Instance::create()
{
	// if (enableValidationLayers && !checkValidationLayerSupport())
	// 	throw std::runtime_error("validation layers requested, but not available!");

	constexpr vk::ApplicationInfo appInfo{
		.pApplicationName = "Hello Triangle",
		.applicationVersion = VK_MAKE_VERSION(1, 0, 0),
		.pEngineName = "No Engine",
		.engineVersion = VK_MAKE_VERSION(1, 0, 0),
		.apiVersion = vk::ApiVersion14};

	auto requiredExtensions = getRequiredExtensions();
	if (!checkGLFWExtensionSupport(requiredExtensions))
		throw std::runtime_error("Required extensions not supported");

	const vk::InstanceCreateInfo createInfo{
		.pApplicationInfo = &appInfo,
		.enabledExtensionCount = static_cast<uint32_t>(requiredExtensions.size()),
		.ppEnabledExtensionNames = requiredExtensions.data()};
	_instance = vk::raii::Instance(_context, createInfo);
}

std::vector<const char *> Instance::getRequiredExtensions()
{
	// Get GLFW extensions needed
	uint32_t glfwExtensionCount = 0;
	auto     glfwExtensions     = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	std::vector<const char *> extensions{
		glfwExtensions,
		glfwExtensions + glfwExtensionCount
	};

	// Add EXT_DEBUG_UTILS extensions IF Validation Layers enable
	if (true) //enableValidationLayers)
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	return extensions;
}

//
// std::vector<VkExtensionProperties> Instance::getInstanceExtensions() {
// 	uint32_t extensionCount = 0;
// 	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
// 	std::vector<VkExtensionProperties> extensions(extensionCount);
// 	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount,
// 	                                       extensions.data());
//
// 	return extensions;
// }
//
// bool Instance::checkExtensionSupport(
// 		std::vector<const char *> requiredExtensions) {
// 	// get available instance extensions
// 	const auto instanceExtensions = getInstanceExtensions();
//
// 	// Check each required extension
// 	for (const char *required: requiredExtensions) {
// 		bool found = false;
// 		for (const auto &ext: instanceExtensions) {
// 			if (std::string(ext.extensionName) == required) {
// 				found = true;
// 				break;
// 			}
// 		}
//
// 		if (!found) {
// 			std::cerr << "Missing required extension: " << required << '\n';
// 			return false; // fail early if any extension is missing
// 		}
// 	}
//
// 	std::cout << "all required extensions have been found!" << std::endl;
// 	return true;
// }
//
// bool Instance::checkValidationLayerSupport() {
// 	// get available layers
// 	uint32_t layerCount;
// 	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
//
// 	std::vector<VkLayerProperties> availableLayers(layerCount);
// 	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());
//
// 	for (const auto &layerProperties: availableLayers) {
// 		std::cout << layerProperties.layerName << std::endl;
// 	}
//
// 	for (const auto layerName: validationLayers) {
// 		bool layerFound = false;
//
// 		for (const auto &layerProperties: availableLayers) {
// 			if (std::string(layerName) == layerProperties.layerName) {
// 				layerFound = true;
// 				break;
// 			}
// 		}
//
// 		if (!layerFound) {
// 			std::cerr << "Validation layer not found: " << layerName << '\n';
// 			return false;
// 		}
// 	}
//
// 	return true;
// }
} // namespace Core::Vulkan
