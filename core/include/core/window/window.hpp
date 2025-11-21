#pragma once

#include "inputState.hpp"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>

#include <string>
#include <thread>

namespace Core {

    struct WindowSpec {
        int width = 1280;
        int height = 720;
        std::string title;
        bool isResizable = true;
        bool vsync = true;
    };

    class Window {
    public:
        Window(const WindowSpec& spec = WindowSpec());
        ~Window();

        void create();
        void destroy();
        void update();
        void pollEvents();

        bool shouldClose();
        glm::vec2 frameBufferSize();
        GLFWwindow* handle() const { return _handle; };

    private:
        WindowSpec _spec;
        GLFWwindow* _handle = nullptr;
    };


}