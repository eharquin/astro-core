//
// Created by eharquin on 12/15/25.
//

// event_queue.hpp
#pragma once
#include <variant>
#include <vector>

#include <core/window/Events.hpp>

namespace Core {

	using WindowEvent = std::variant<
		WindowResizeEvent,
		WindowMoveEvent,
		WindowFocusEvent,
		WindowMinimizeEvent,
		WindowCloseEvent
	>;

	class EventQueue {
	public:
		void push(WindowEvent event) {
			_events.emplace_back(std::move(event));
		}

		std::vector<WindowEvent> drain() {
			auto events = std::move(_events);
			_events.clear();
			return events;
		}

	private:
		std::vector<WindowEvent> _events;
	};

} // namespace Core
