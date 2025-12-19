//
// Created by eharquin on 12/19/25.
//
// modelUtils.hpp

#pragma once

#include <tiny_obj_loader.h>

#include <unordered_map>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>

namespace Core {
	template<typename V>
	struct ModelData {
		std::vector<V> vertices;
		std::vector<uint32_t> indices;
	};

	template<typename V>
	ModelData<V> loadModel(const std::string &filename) {
		ModelData<V> modelData;

		tinyobj::attrib_t                attrib;
		std::vector<tinyobj::shape_t>    shapes;
		std::vector<tinyobj::material_t> materials;
		std::string                      warn, err;

		if (!LoadObj(&attrib, &shapes, &materials, &warn, &err, filename.c_str()))
			throw std::runtime_error(warn + err);

		std::unordered_map<V, uint32_t> uniqueVertices{};

		for (const auto &shape : shapes)
		{
			for (const auto &index : shape.mesh.indices)
			{
				V vertex{};

				vertex.pos = {
					attrib.vertices[3 * index.vertex_index + 0],
					attrib.vertices[3 * index.vertex_index + 1],
					attrib.vertices[3 * index.vertex_index + 2]};

				vertex.texCoord = {
					attrib.texcoords[2 * index.texcoord_index + 0],
					1.0f - attrib.texcoords[2 * index.texcoord_index + 1]};

				vertex.color = {1.0f, 1.0f, 1.0f};

				if (!uniqueVertices.contains(vertex))
				{
					uniqueVertices[vertex] = static_cast<uint32_t>(modelData.vertices.size());
					modelData.vertices.push_back(vertex);
				}

				modelData.indices.push_back(uniqueVertices[vertex]);
			}
		}

		return modelData;
	}
}
