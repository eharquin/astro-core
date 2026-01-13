//
// Created by eharquin on 12/26/25.
//

#pragma once

#include <core/window/IWindowContext.hpp>

namespace Core::Window::GLFW {

	class WindowContext : public IWindowContext {
	public:
		WindowContext();
		~WindowContext() override;

		// Create a window from a spec
		std::unique_ptr<IWindow> createWindow(const WindowSpec& spec) override;

		// 🔑 Vulkan support
		bool supportsVulkan() const override { return true; }
		VkSurfaceKHR createVulkanSurface(void* instance, IWindow& window) const override;
		std::vector<const char *> getVulkanRequiredExtensions() const override;
	};

} // namespace Core