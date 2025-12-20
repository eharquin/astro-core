#pragma once

#include <core/app/Layer.hpp>


#include <core/rendering/Renderer.hpp>
#include <core/rendering/Model.hpp>


class SimpleModelLayer : public Core::Layer {
public:
	SimpleModelLayer(Core::Rendering::IRenderer& renderer)
		: _renderer(renderer) {

		const auto model = Core::Rendering::Model("../../models/viking_room/viking_room.obj","../../models/viking_room/viking_room.png");
		_renderer.addRenderable(model);
	}


private:
	Core::Rendering::IRenderer& _renderer;
};