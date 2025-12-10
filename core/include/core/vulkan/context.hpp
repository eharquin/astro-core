//
// Created by eharquin on 12/7/25.
//

#pragma once

#if defined(__INTELLISENSE__) || !defined(USE_CPP20_MODULES)
#include <vulkan/vulkan_raii.hpp>
#else
import vulkan_hpp;
#endif

#include <vector>
#include <optional>

namespace Core::Vulkan {
#ifdef NDEBUG
	constexpr bool enableValidationLayers = false;
#else
	constexpr bool enableValidationLayers = true;
#endif

	const std::vector<const char *> validationLayers = {
		"VK_LAYER_KHRONOS_validation",
	};

	const std::vector<const char *> deviceExtensions = {
		vk::KHRSwapchainExtensionName,
		vk::KHRSpirv14ExtensionName,
		vk::KHRSynchronization2ExtensionName,
		vk::KHRCreateRenderpass2ExtensionName
	};


	class Context {
	public:
		Context() = default;

		~Context() = default;

		Context(const Context &) = delete;

		Context &operator=(const Context &) = delete;

		Context(Context &&) = delete;

		Context &operator=(Context &&) = delete;

		void create();

	private:
		void createInstance();

		static std::vector<const char *> getRequiredExtensions();

		[[nodiscard]] bool checkExtensionSupport(const std::vector<const char *> &requiredExtensions) const;

		[[nodiscard]] bool checkLayerSupport(const std::vector<const char *> &requiredLayers) const;

		static vk::Bool32 debugCallback(vk::DebugUtilsMessageSeverityFlagBitsEXT severity,
		                                vk::DebugUtilsMessageTypeFlagsEXT type,
		                                const vk::DebugUtilsMessengerCallbackDataEXT *
		                                pCallbackData, void *);

		void setupDebugMessenger();

		void pickPhysicalDevice();


		void createLogicalDevice();

		// Instance
		vk::raii::Context _context;
		vk::raii::Instance _instance = nullptr;
		vk::raii::DebugUtilsMessengerEXT _debugMessenger = nullptr;
		// Device
		vk::raii::PhysicalDevice _physicalDevice = nullptr;
		vk::raii::Device _device = nullptr;
		vk::raii::Queue _graphicsQueue = nullptr;
	};
}
