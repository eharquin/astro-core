#include <core/app/App.hpp>

#include <utility>

#include "core/rendering/ContextFactory.hpp"
#include "core/window/WindowFactory.hpp"

namespace Core::App
{
App::App(AppSpec spec) :
	_spec(std::move(spec)) {
	_app = this;
}

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
	_windowContext = Window::createWindowContext(_spec.windowSpec.api);
	_window = _windowContext->createWindow(_spec.windowSpec);
}

void App::initGraphics()
{
	_graphicsContext = Rendering::createGraphicsContext(_spec.graphicsAPI);
	_graphicsContext->init(*_windowContext, *_window);
	_renderer = _graphicsContext->createRenderer(*_window);
}

void App::mainloop()
{
	for (auto& layer : _layers)
		layer->onAttach(); // une seule fois après initGraphics


	float lastTime = time();
	while (_running)
	{
		_windowContext->pollEvents();

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
	_graphicsContext->shutdown();
}

void App::cleanup()
{
}

float App::time()
{
	return static_cast<float>(glfwGetTime());
}
} // namespace Core