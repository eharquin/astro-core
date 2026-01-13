//
// Created by eharquin on 12/27/25.
//

#pragma once

namespace Core::Window {

	enum class WindowsAPI {
		None,
		GLFW,
		SDL,
		Wayland,
		Headless
	};

}