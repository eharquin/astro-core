//
// Created by eharquin on 12/26/25.
//

#pragma once

#include <string>
#include <string_view>
#include <glm/glm.hpp>

#include "EventQueue.hpp"
#include "WindowsAPI.hpp"

struct VkSurfaceKHR_T; // forward declaration
using VkSurfaceKHR = VkSurfaceKHR_T*;

namespace Core::Window {

	struct WindowSpec {
		int width  = 1280;
		int height = 720;

		std::string title = "Window";
		bool resizable = true;

		// Which windowing API to use (default is GLFW)
		WindowsAPI api = WindowsAPI::GLFW;
	};

	struct WindowState {
		glm::vec2 framebufferSize{0, 0};
		glm::vec2 lastFramebufferSize{0, 0};

		glm::vec2 windowSize{0, 0};
		glm::vec2 windowPosition{0, 0};

		float contentScale = 1.0f;

		bool focused   = true;
		bool minimized = false;
		bool maximized = false;
		bool shouldClose = false;

		bool framebufferResized = false;
	};


	class IWindow {
	public:
		virtual ~IWindow() = default;

		virtual void update() = 0;

		// 🔑 Window state access
		virtual const WindowState& state() const = 0;
		virtual void resetFramebufferResized() = 0;

		virtual bool shouldClose() const = 0;

		virtual void setTitle(std::string_view title) = 0;

		// 🔑 Native handle abstraction
		virtual void* nativeHandle() const = 0;

		// 🔑 Event queue access
		virtual EventQueue& events() = 0;
	};

} // namespace Core