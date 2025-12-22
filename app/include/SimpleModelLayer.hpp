#pragma once

#include <core/app/App.hpp>
#include <core/app/Layer.hpp>

#include <core/utils/MeshUtils.hpp>
#include "core/utils/ImageUtils.hpp"
#include <core/utils/FileUtils.hpp>

using namespace Core::App;

class SimpleModelLayer : public Layer {
public:
	SimpleModelLayer() = default;

	void onAttach() override {
		auto renderer = App::instance()->renderer();
		auto shaderData = Core::Utils::readShader("../../shaders/slang.spv");
		renderer->createPipeline("basic", shaderData);

		auto meshData = Core::Utils::loadMesh("../../models/viking_room/viking_room.obj");
		auto meshID = renderer->createMesh(meshData);
		auto textureData = Core::Utils::readTexture("../../models/viking_room/viking_room.png");
		auto textureID = renderer->createTexture(textureData);
		renderer->addInstance(meshID, textureID);
	}
};