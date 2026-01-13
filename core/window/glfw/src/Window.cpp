//
// Created by eharquin on 12/26/25.
//

#include <stdexcept>
#include <core/window/glfw/Window.hpp>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace Core::Window::GLFW {

	Window::Window(const WindowSpec &spec)
	{
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

		if (!spec.resizable)
			glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

		_window = glfwCreateWindow(
			spec.width,
			spec.height,
			spec.title.c_str(),
			nullptr,
			nullptr
		);

		if (!_window)
			throw std::runtime_error("Failed to create GLFW window");

		glfwSetWindowUserPointer(_window, this);
		glfwSetFramebufferSizeCallback(_window, framebufferResizeCallback);
		glfwSetWindowFocusCallback(_window, windowFocusCallback);
		glfwSetWindowIconifyCallback(_window, windowIconifyCallback);
		glfwSetWindowCloseCallback(_window, windowCloseCallback);
		glfwSetWindowPosCallback(_window, windowMoveCallback);

		// Initialize state
		int w, h;
		glfwGetFramebufferSize(_window, &w, &h);
		_state.framebufferSize = { float(w), float(h) };
		_state.lastFramebufferSize = _state.framebufferSize;
		_state.windowSize = { float(spec.width), float(spec.height) };
		_state.shouldClose = false;
	}

	Window::~Window()
	{
		if (_window) {
			glfwDestroyWindow(_window);
			_window = nullptr;
		}
	}

	void Window::update() {
		// Currently no per-frame update logic needed
	}


	void Window::setTitle(std::string_view title) { glfwSetWindowTitle(_window, title.data()); }

	void Window::framebufferResizeCallback(GLFWwindow* window, int w, int h)
	{
		auto* self = static_cast<Window*>(glfwGetWindowUserPointer(window));

		self->_state.lastFramebufferSize = self->_state.framebufferSize;
		self->_state.framebufferSize = { float(w), float(h) };
		self->_state.framebufferResized = true;

		self->_eventQueue.push(WindowResizeEvent{ glm::vec2{w, h} });
	}

	void Window::windowFocusCallback(GLFWwindow* window, int focused)
	{
		auto* self = static_cast<Window*>(glfwGetWindowUserPointer(window));
		self->_state.focused = (focused == GLFW_TRUE);
		self->_eventQueue.push(WindowFocusEvent{ self->_state.focused });
	}

	void Window::windowIconifyCallback(GLFWwindow* window, int iconified)
	{
		auto* self = static_cast<Window*>(glfwGetWindowUserPointer(window));
		self->_state.minimized = (iconified == GLFW_TRUE);
		self->_eventQueue.push(WindowMinimizeEvent{ self->_state.minimized });
	}

	void Window::windowCloseCallback(GLFWwindow* window)
	{
		auto* self = static_cast<Window*>(glfwGetWindowUserPointer(window));
		self->_state.shouldClose = true;
		self->_eventQueue.push(WindowCloseEvent{});
	}

	void Window::windowMoveCallback(GLFWwindow* window, int xpos, int ypos)
	{
		auto* self = static_cast<Window*>(glfwGetWindowUserPointer(window));
		self->_state.windowPosition = { float(xpos), float(ypos) };
		self->_eventQueue.push(WindowMoveEvent{ self->_state.windowPosition });
	}
}
