#pragma once

#include "inputState.hpp"


#include <string>
#include <string_view>
#include <cstdint>

#include <glm/glm.hpp>

#include <core/window/eventQueue.hpp>

struct GLFWwindow; // forward declaration

namespace Core {

struct WindowSpec {
    int width  = 1280;
    int height = 720;

    std::string title = "Window";

    bool resizable = true;
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

    bool framebufferResized = false;
};

class Window {
public:
    explicit Window(const WindowSpec& spec = WindowSpec());
    ~Window();

    // ---- not copyable ----
    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;

    // ---- movable ----
    Window(Window&& other) noexcept;
    Window& operator=(Window&& other) noexcept;

    void update();
    void pollEvents() const;

    [[nodiscard]] bool shouldClose() const noexcept;
    [[nodiscard]] bool isMinimized() const noexcept;
    [[nodiscard]] bool isFocused() const noexcept;

    void setTitle(std::string_view title);

    [[nodiscard]] glm::vec2 framebufferSize() const noexcept;

    [[nodiscard]] bool framebufferResized() const noexcept;
    void resetFramebufferResized() noexcept;

    [[nodiscard]] void* nativeHandle() const noexcept { return _glfwHandle; }
    [[nodiscard]] GLFWwindow* glfwHandle() const noexcept { return _glfwHandle; }

private:
    WindowSpec _spec;
    GLFWwindow* _glfwHandle = nullptr;
    EventQueue _events;
    WindowState _state;

    static void framebufferResizeCallback(GLFWwindow* window, int width, int height);
    static void windowFocusCallback(GLFWwindow* window, int focused);
    static void windowIconifyCallback(GLFWwindow* window, int iconified);
    static void windowCloseCallback(GLFWwindow* window);
};

}