#include <core/app/App.hpp>

#include <iostream>

namespace Core
{
App::App(const AppSpec &spec) :
	_spec(spec) {
	_app = this;
}

App::~App()
{}

void App::run()
{
	_running = true;
	initWindow();
	initGraphics();
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

void App::initGraphics()
{
	// Create graphics context based on spec
	_context = Rendering::createContext(_spec.graphicsAPI);
	if (!_context)
		throw std::runtime_error("Failed to create graphics context");

	// Initialize context with the window
	_context->init(*_window);

	// Create renderer
	_renderer = _context->createRenderer(*_window);
	_renderer->init();
}

void App::mainloop()
{
	for (auto& layer : _layers)
		layer->onAttach(); // une seule fois après initGraphics


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

		_renderer->drawFrame();

		// NOTE: rendering can be done elsewhere (eg. render thread)
		for (const auto &layer : _layers)
			layer->onRender();

		_window->update();
	}

	_renderer->shutdown();
	_context->shutdown();
}

void App::cleanup()
{
}

float App::time()
{
	return static_cast<float>(glfwGetTime());
}
} // namespace Core