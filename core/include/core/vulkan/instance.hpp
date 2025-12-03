#pragma once

#if defined(__INTELLISENSE__) || !defined(USE_CPP20_MODULES)
#include <vulkan/vulkan_raii.hpp>
#else
import vulkan_hpp;
#endif

#include <vector>

namespace Core::Vulkan {
	class Instance {
	public:
		Instance() = default;

		void create();

	private:
		static std::vector<const char *> getRequiredExtensions();

		[[nodiscard]] bool checkExtensionSupport(const std::vector<const char *> &requiredExtensions) const;

		[[nodiscard]] bool checkLayerSupport(const std::vector<const char *> &requiredLayers) const;

		static vk::Bool32 debugCallback(vk::DebugUtilsMessageSeverityFlagBitsEXT severity,
		                                vk::DebugUtilsMessageTypeFlagsEXT type,
		                                const vk::DebugUtilsMessengerCallbackDataEXT *
		                                pCallbackData, void *);

		void setupDebugMessenger();

		vk::raii::Context _context;
		vk::raii::Instance _instance = nullptr;
		vk::raii::DebugUtilsMessengerEXT _debugMessenger = nullptr;
	};
} // namespace Core::Vulkan
