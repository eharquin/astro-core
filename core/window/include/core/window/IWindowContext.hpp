//
// Created by eharquin on 12/26/25.
//

 #pragma once
#include <memory>
#include "IWindow.hpp"

struct VkSurfaceKHR_T; // forward declaration
using VkSurfaceKHR = VkSurfaceKHR_T*;

namespace Core::Window {

	class IWindowContext {
	public:
		virtual ~IWindowContext() = default;

		// Create a window from a spec
		virtual std::unique_ptr<IWindow> createWindow(const WindowSpec& spec) = 0;

		// Poll for events
		void pollEvents();

		// 🔑 Vulkan support
		virtual bool supportsVulkan() const { return true; }
		virtual VkSurfaceKHR createVulkanSurface(void* instance, const IWindow& window) const;
		virtual std::vector<const char *> getVulkanRequiredExtensions() const;
	};

} // namespace Core
