#pragma once

#include "Event.hpp"

namespace Core {
class Layer {
  public:
	virtual ~Layer() = default;

	virtual void onEvent(Event &event) {}
	virtual void onUpdate(float dt) {}
	virtual void onRender() {}
};
} // namespace Core
