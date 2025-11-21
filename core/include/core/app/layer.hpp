#pragma once

#include "event.hpp"

namespace Core {
    class Layer {
    public:
        virtual ~Layer() = default;

        virtual void onEvent(event& event) {}
        virtual void onUpdate(float dt) {}
        virtual void onRender() {}
    };
}