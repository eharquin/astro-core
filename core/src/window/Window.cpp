#include <iostream>
#include <core/window/Window.hpp>

#include <stdexcept>
#include <string>

namespace Core {

	Window::Window(const WindowSpec &spec)
		: _spec(spec)
	{
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

		if (!_spec.resizable)
			glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

		_glfwHandle = glfwCreateWindow(_spec.width, _spec.height, _spec.title.data(), nullptr, nullptr);

		glfwSetWindowUserPointer(_glfwHandle, this);
		glfwSetFramebufferSizeCallback(_glfwHandle, framebufferResizeCallback);

		int w, h;
		glfwGetFramebufferSize(_glfwHandle, &w, &h);
		_state.framebufferSize = { w, h };
		_state.lastFramebufferSize = _state.framebufferSize;
	}

	Window::~Window()
	{
		glfwDestroyWindow(_glfwHandle);
		_glfwHandle = nullptr;
	}

	Window::Window(Window &&other) noexcept {
		_spec = other._spec;
		_glfwHandle = other._glfwHandle;
		other._glfwHandle = nullptr;
	}

	Window & Window::operator=(Window &&other) noexcept {
		if (this != &other) {
			_spec = other._spec;
			_glfwHandle = other._glfwHandle;
			other._glfwHandle = nullptr;
		}
		return *this;
	}

	void Window::update() {
	}

	void Window::pollEvents() const { glfwPollEvents(); }

	bool Window::shouldClose() const noexcept { return glfwWindowShouldClose(_glfwHandle) != 0; }

	bool Window::isMinimized() const noexcept {
		return _state.minimized;
	}

	bool Window::isFocused() const noexcept {
		return _state.focused;
	}

	void Window::setTitle(std::string_view title) {
		_spec.title = title;
		glfwSetWindowTitle(_glfwHandle, title.data());
	}

	glm::vec2 Window::framebufferSize() const noexcept {
		return _state.framebufferSize;
	}

	bool Window::framebufferResized() const noexcept {
		return _state.framebufferResized;
	}

	void Window::resetFramebufferResized() noexcept {
		_state.framebufferResized = false;
	}


	void Window::framebufferResizeCallback(GLFWwindow *glfwHandle, int width, int height) {
		auto* self = static_cast<Window*>(glfwGetWindowUserPointer(glfwHandle));
		if (!self) return;

		std::cout << "[ASTRO CORE] [WINDOW] Framebuffer resized to "
				  << width << "x" << height << std::endl;

		self->_state.lastFramebufferSize = self->_state.framebufferSize;
		self->_state.framebufferSize = { width, height };
		self->_state.framebufferResized = true;

		self->_events.push(WindowResizeEvent{ self->_state.framebufferSize });
	}

	void Window::windowFocusCallback(GLFWwindow *window, int focused) {
		auto* self = static_cast<Window*>(glfwGetWindowUserPointer(window));
		if (!self) return;

		self->_state.focused = (focused != 0);

		self->_events.push(WindowFocusEvent{ self->_state.focused });
	}

	void Window::windowIconifyCallback(GLFWwindow *window, int iconified) {
		auto* self = static_cast<Window*>(glfwGetWindowUserPointer(window));
		if (!self) return;

		self->_state.minimized = (iconified != 0);

		self->_events.push(WindowMinimizeEvent{ self->_state.minimized });
	}

	void Window::windowCloseCallback(GLFWwindow *window) {
		auto* self = static_cast<Window*>(glfwGetWindowUserPointer(window));
		if (!self) return;

		self->_events.push(WindowCloseEvent{});
	}

} // namespace Core
