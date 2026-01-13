//
// Created by eharquin on 12/26/25.
//

#include <core/window/glfw/WindowContext.hpp>
#include <core/window/glfw/Window.hpp>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace Core::Window::GLFW {
	WindowContext::WindowContext() {
		glfwInit();
	}

	WindowContext::~WindowContext() {
		glfwTerminate();
	}

	std::unique_ptr<IWindow> WindowContext::createWindow(const WindowSpec& spec) {
		return std::make_unique<Window>(spec);
	}

	VkSurfaceKHR WindowContext::createVulkanSurface(void* instance, IWindow& window) const {
		VkSurfaceKHR surface;
		if (glfwCreateWindowSurface(static_cast<VkInstance>(instance), static_cast<GLFWwindow *>(window.nativeHandle()), nullptr, &surface) != GLFW_TRUE) {
			throw std::runtime_error("Failed to create Vulkan surface");
		}
		return surface;
	}

	std::vector<const char *> WindowContext::getVulkanRequiredExtensions() const {
		uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
		return std::vector(glfwExtensions, glfwExtensions + glfwExtensionCount);
	}

}