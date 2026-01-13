//
// Created by eharquin on 12/15/25.
//

// window_events.hpp
#pragma once
#include <variant>
#include <glm/glm.hpp>

namespace Core {

	struct WindowResizeEvent {
		glm::vec2 framebufferSize;
	};

	struct WindowMoveEvent {
		glm::vec2 position;
	};

	struct WindowFocusEvent {
		bool focused;
	};

	struct WindowMinimizeEvent {
		bool minimized;
	};

	struct WindowCloseEvent {};

	using WindowEvent = std::variant<
		WindowResizeEvent,
		WindowMoveEvent,
		WindowFocusEvent,
		WindowMinimizeEvent,
		WindowCloseEvent
	>;


} // namespace Core