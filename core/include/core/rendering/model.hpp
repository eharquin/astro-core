//
// Created by eharquin on 12/19/25.
//


#pragma once

#include <cstdint>
#include <string>
#include <utility>
#include <vector>

#include <core/utils/modelUtils.hpp>

namespace Core {
	template<typename V>
	class Model {
	public:
		Model() = default;
		Model(std::string modelPath, std::string  texturePath)
		: _modelPath(std::move(modelPath)), _texturePath(std::move(texturePath)) {
			auto modelData = loadModel<V>(_modelPath);
			_vertices.swap(modelData.vertices);
			_indices.swap(modelData.indices);
		}
		~Model() = default;

		const std::vector<V>& vertices() const noexcept { return _vertices; }
		const std::vector<uint32_t>& indices() const noexcept { return _indices; }

	private:
		std::string _modelPath;
		std::string _texturePath;
		std::vector<V> _vertices;
		std::vector<uint32_t> _indices;
	};
}