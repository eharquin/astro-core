#pragma once

#include "Event.hpp"

namespace Core::App {
class Layer {
  public:
	virtual ~Layer() = default;

	virtual void onAttach() {}
	virtual void onEvent(Event &event) {}
	virtual void onUpdate(float dt) {}
	virtual void onRender() {}
};
} // namespace Core
