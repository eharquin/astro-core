#pragma once

#include <core/rendering/model.hpp>
#include <core/rendering/vulkan/vertex.hpp>

#include <core/app/layer.hpp>

class SimpleModelLayer : public Core::Layer {
public:
	SimpleModelLayer() = default;

private:
	const std::string _modelPath = "../../models/viking_room/viking_room.obj";
	const std::string _texturePath = "../../models/viking_room/viking_room.png";
	Core::Model<Core::Vulkan::Vertex> model{_modelPath, _texturePath};
};