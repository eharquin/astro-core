#include <core/app/App.hpp>

#include <iostream>

namespace Core
{
App::App(const AppSpec &spec) :
	_spec(spec)
{}

App::~App()
{}

void App::run()
{
	_running = true;
	initWindow();
	initVulkan();
	mainloop();
	cleanup();
}

void App::initWindow()
{
	_window = std::make_shared<Window>(_spec.windowSpec);

	std::cout << "[ASTRO CORE] [APP] [INIT] Window (GLFW) created"
				<< "(w=" << _spec.windowSpec.width
				<< ", h=" << _spec.windowSpec.height << ")"
				<< std::endl;
}

void App::initVulkan()
{
	_context.create(*_window);

	std::cout << "[ASTRO CORE] [APP] [INIT] Vulkan context initialized (validation_layer=ON)" << std::endl;
}

void App::mainloop()
{
	float lastTime = time();
	while (_running)
	{
		_window->pollEvents();

		if (_window->shouldClose())
			_running = false;

		float currentTime = time();
		float deltaTime   = currentTime - lastTime;
		lastTime          = currentTime;

		for (const auto &layer : _layers)
			layer->onUpdate(deltaTime);

		_context.drawFrame(_window->width(), _window->height());

		// NOTE: rendering can be done elsewhere (eg. render thread)
		for (const auto &layer : _layers)
			layer->onRender();

		_window->update();
	}

	_context.stop();
}

void App::cleanup()
{
}

float App::time()
{
	return static_cast<float>(glfwGetTime());
}
} // namespace Core