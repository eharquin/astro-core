#include <core/window/window.hpp>

#include <stdexcept>
#include <string>
#include <iostream>

namespace Core {

    static void GLFWErrorCallback(int error, const char* description) {
        throw std::runtime_error(std::string("GLFW Error: ") + description);
    }

    Window::Window(const WindowSpec& spec)
        : _spec(spec) {
    }

    Window::~Window() {
        destroy();
    }

    void Window::create() {
        glfwSetErrorCallback(GLFWErrorCallback);

        glfwInit();

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
        
        _handle = glfwCreateWindow(800, 600, "Vulkan", nullptr, nullptr);
    }

    void Window::destroy() {
        glfwDestroyWindow(_handle);
        _handle = nullptr;
        glfwTerminate();
    }

    void Window::update() {

    }

    void Window::pollEvents()
    {
        glfwPollEvents();
    }

    bool Window::shouldClose() {
        return glfwWindowShouldClose(_handle) != 0;
    }

    glm::vec2 Window::frameBufferSize() {
        int width, height;
        glfwGetFramebufferSize(_handle, &width, &height);
        return { width, height };
    }
}

