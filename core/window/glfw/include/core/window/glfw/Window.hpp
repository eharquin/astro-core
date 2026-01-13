//
// Created by eharquin on 12/26/25.
//
#pragma once

#include <core/window/IWindow.hpp>

struct GLFWwindow; // forward declaration

namespace Core::Window::GLFW {

	class Window final : public IWindow {
	public:
		explicit Window(const WindowSpec& spec);
		~Window() override;

		void update() override;

		const WindowState& state() const override { return _state; }
		void resetFramebufferResized() override { _state.framebufferResized = false; }

		void setTitle(std::string_view title) override;

		bool shouldClose() const override { return _state.shouldClose; }

		void* nativeHandle() const override { return _window; }

		EventQueue& events() override { return _eventQueue; }

	private:
		GLFWwindow* _window = nullptr;
		WindowState _state;
		EventQueue _eventQueue;

		static void framebufferResizeCallback(GLFWwindow* window, int w, int h);
		static void windowFocusCallback(GLFWwindow* window, int focused);
		static void windowIconifyCallback(GLFWwindow* window, int iconified);
		static void windowCloseCallback(GLFWwindow* window);
		static void windowMoveCallback(GLFWwindow* window, int xpos, int ypos);
	};

} // namespace Core