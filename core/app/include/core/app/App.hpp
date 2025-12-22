#pragma once

#include <memory>
#include <string>
#include <vector>

#include <core/window/GLFWContext.hpp>
#include <core/window/Window.hpp>
#include <core/app/api.hpp>

#include "Layer.hpp"

namespace Core::App
{
static GLFWContext g_glfwContext;

struct AppSpec {
	std::string name = "Application";
	WindowSpec windowSpec;
	GraphicsAPI graphicsAPI = GraphicsAPI::Vulkan;
};

class App {
public:
	App(const AppSpec &spec = AppSpec());
	~App();

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

	std::shared_ptr<Window> _window;
	std::unique_ptr<Rendering::IContext> _context;
	std::unique_ptr<Rendering::IRenderer> _renderer;

	bool _running = false;
	std::vector<std::unique_ptr<Layer> > _layers;

	static inline App* _app = nullptr;
};
}
