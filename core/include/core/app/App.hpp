#pragma once

#include <memory>
#include <string>
#include <vector>

#include <core/window/GLFWContext.hpp>
#include <core/window/Window.hpp>
#include <core/rendering/vulkan/Context.hpp>

#include "Layer.hpp"


namespace Core
{

static GLFWContext glfwContext;

/**
 * @brief Specification used to configure an application instance.
 *
 * This struct contains all user-configurable parameters passed to
 * the Core::App constructor, including window properties and the
 * application name.
 */
struct AppSpec
{
	/// Human-readable name of the application.
	std::string name = "Application";

	/// Window creation parameters.
	WindowSpec windowSpec;
};

/**
 * @brief Main application class.
 *
 * This class owns the main window, the Vulkan instance, and the list
 * of active layers. It is responsible for:
 * - initializing the engine (window + Vulkan)
 * - running the main loop
 * - managing layers in a stack-like structure
 */
class App
{
public:
	/**
	 * @brief Construct a new application instance.
	 *
	 * @param spec Configuration parameters (name, window spec, etc.)
	 */
	App(const AppSpec &spec = AppSpec());

	/// @brief Destroy the application, releasing all resources.
	~App();

	/**
	 * @brief Runs the application's main loop.
	 *
	 * This call blocks until the user closes the window or
	 * requests termination.
	 */
	void run();

	/**
	 * @brief Adds a new layer to the application.
	 *
	 * @tparam TLayer A class deriving from Core::Layer.
	 *
	 * Example:
	 * @code
	 * app.pushLayer<MyGameLayer>();
	 * @endcode
	 */
	template <typename TLayer>
		requires(std::is_base_of_v<Layer, TLayer>)
	void pushLayer()
	{
		_layers.push_back(std::make_unique<TLayer>());
	}

	/**
	 * @brief Returns the elapsed time since the application started.
	 *
	 * @return Time in seconds as a float.
	 */
	static float time();

private:
	// --- Internals --------------------------------------------------------

	/// Application configuration.
	AppSpec _spec;

	/// The main application window.
	std::shared_ptr<Window> _window;

	/// True while the application mainloop is running.
	bool _running = false;

	/// Stack of active layers.
	std::vector<std::unique_ptr<Layer> > _layers;

	// --- Internal setup steps --------------------------------------------

	/// @brief Creates the window based on AppSpec.
	void initWindow();

	/// @brief Creates and initializes the Vulkan instance.
	void initVulkan();

	/// @brief Runs one iteration of the main loop.
	void mainloop();

	/// @brief Frees all allocated resources.
	void cleanup();

	// --- Vulkan -----------------------------------------------------------

	/// Vulkan instance used by the application.
	Rendering::Vulkan::Context _context;

};
}
