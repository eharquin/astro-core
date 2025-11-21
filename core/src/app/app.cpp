#include <core/app/app.hpp>

namespace Core {


    App::App(const AppSpec& spec)
        : _spec(spec) {
        
    }

    App::~App() {

    }

    void App::run() {
        _running = true;
        initWindow();
        initVulkan();
        mainloop();
        cleanup();
    }


    void App::initWindow() {
        _window = std::make_shared<Window>(_spec.windowSpec);
        _window->create();
    }

    void App::initVulkan() {

    }

    void App::mainloop() {
        float lastTime = time();
        while(_running) {
            _window->pollEvents();

            if(_window->shouldClose())
                _running = false;

            float currentTime = time();
            float deltaTime = currentTime - lastTime;
            lastTime = currentTime;

            for (const auto& layer : _layers )
                layer->onUpdate(deltaTime);

            // NOTE: rendering can be done elsewhere (eg. render thread)
            for (const auto& layer : _layers)
                layer->onRender();
            

            _window->update();
        }
    }

    void App::cleanup() {
        _window->destroy();
    }

    float App::time() {
        return static_cast<float>(glfwGetTime());
    }
}