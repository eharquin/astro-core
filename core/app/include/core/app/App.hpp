#pragma once

#include <memory>
#include <string>
#include <vector>

#include <core/window/IWindowContext.hpp>
#include <core/rendering/GraphicsAPI.hpp>
#include <core/window/IWindow.hpp>

#include "Layer.hpp"
#include "core/rendering/IContext.hpp"
#include "core/rendering/IRenderer.hpp"

namespace Core::App
{

struct AppSpec {
	std::string name = "Application";
	Window::WindowSpec windowSpec;
	Rendering::GraphicsAPI graphicsAPI = Rendering::GraphicsAPI::Vulkan;
};

class App {
public:
	explicit App(AppSpec spec = AppSpec());
	~App() = default;

	void run();

	template <typename TLayer, typename... Args>
	requires(std::is_base_of_v<Layer, TLayer>)
	void pushLayer(Args&&... args) {
		_layers.push_back(std::make_unique<TLayer>(std::forward<Args>(args)...));
	}

	static float time();

	static App* instance() { return _app; }
	Rendering::IRenderer* renderer() { return _renderer.get(); }

private:
	void initWindow();
	void initGraphics();
	void mainloop();
	void cleanup();

	AppSpec _spec;

	std::shared_ptr<Window::IWindowContext> _windowContext;
	std::unique_ptr<Window::IWindow> _window;

	std::unique_ptr<Rendering::IContext> _graphicsContext;
	std::unique_ptr<Rendering::IRenderer> _renderer;

	bool _running = false;
	std::vector<std::unique_ptr<Layer> > _layers;

	static inline App* _app = nullptr;
};
}
