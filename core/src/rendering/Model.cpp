//
// Created by eharquin on 12/20/25.
//

#include <core/rendering/Model.hpp>
#include <core/utils/ModelUtils.hpp>


Core::Rendering::Model::Model(std::string modelPath, std::string texturePath): _modelPath(std::move(modelPath)), _texturePath(std::move(texturePath)) {
	auto modelData = Utils::loadModel<Vertex>(_modelPath);
	_vertices.swap(modelData.vertices);
	_indices.swap(modelData.indices);
}