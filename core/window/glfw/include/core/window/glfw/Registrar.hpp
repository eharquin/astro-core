//
// Created by eharquin on 12/27/25.
//

#pragma once

#include <core/window/WindowFactory.hpp>
#include <core/window/glfw/WindowContext.hpp>

#include "WindowContext.hpp"

namespace Core::Window::GLFW {
	struct GlfwRegistrar {
		GlfwRegistrar() {
			Window::registerWindowContextFactory(
				WindowsAPI::GLFW,[] { return std::make_unique<WindowContext>(); }
			);
		}
	};

	static GlfwRegistrar registrar;
}
