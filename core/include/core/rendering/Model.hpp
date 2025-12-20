//
// Created by eharquin on 12/19/25.
//


#pragma once

#include <cstdint>
#include <string>
#include <utility>
#include <vector>


#include <core/rendering/Vertex.hpp>

namespace Core::Rendering {
	class Model {
	public:
		Model() = default;
		Model(std::string modelPath, std::string  texturePath);

		~Model() = default;

		const std::vector<Vertex>& vertices() const noexcept { return _vertices; }
		const std::vector<uint32_t>& indices() const noexcept { return _indices; }

	private:
		std::string _modelPath;
		std::string _texturePath;
		std::vector<Vertex> _vertices;
		std::vector<uint32_t> _indices;
	};
}
