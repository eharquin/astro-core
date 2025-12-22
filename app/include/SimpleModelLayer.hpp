#pragma once

#include <core/app/App.hpp>
#include <core/app/Layer.hpp>


#include <core/rendering/IRenderer.hpp>
#include <core/rendering/Model.hpp>

#include "core/utils/ModelUtils.hpp"

using namespace Core;

class SimpleModelLayer : public Layer {
public:
	SimpleModelLayer() = default;

	void onAttach() override {
		auto renderer = Core::App::instance()->renderer();

		auto model = Utils::loadModel("../../models/viking_room/viking_room.obj");
		auto meshID = renderer->createMesh(model.vertices, model.indices);
		auto textureID = renderer->createTexture("../../models/viking_room/viking_room.png");
		renderer->addInstance(meshID, textureID);
	}
};