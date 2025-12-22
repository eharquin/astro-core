//
// Created by eharquin on 12/19/25.
//

#include <core/utils/ModelUtils.hpp>

Core::Utils::ModelData Core::Utils::loadModel(const std::string &filename) {
	ModelData modelData;

	tinyobj::attrib_t                attrib;
	std::vector<tinyobj::shape_t>    shapes;
	std::vector<tinyobj::material_t> materials;
	std::string                      warn, err;

	if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filename.c_str()))
		throw std::runtime_error(warn + err);

	std::unordered_map<Rendering::Vertex, uint32_t> uniqueVertices{};

	for (const auto &shape : shapes)
	{
		for (size_t f = 0; f < shape.mesh.indices.size(); f += 3) {
			std::array<uint32_t,3> faceIndices;

			for (size_t k = 0; k < 3; ++k) {
				auto index = shape.mesh.indices[f + k];
				Rendering::Vertex vertex{};

				// position
				vertex.pos = {
					attrib.vertices[3 * index.vertex_index + 0],
					attrib.vertices[3 * index.vertex_index + 1],
					attrib.vertices[3 * index.vertex_index + 2]
				};

				// texcoord
				if (index.texcoord_index >= 0)
					vertex.texCoord = {
						attrib.texcoords[2 * index.texcoord_index + 0],
						1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
					};
				else
					vertex.texCoord = {0.0f, 0.0f};

				// color
				vertex.color = {1.0f, 1.0f, 1.0f};

				// normal
				if (index.normal_index >= 0)
					vertex.normal = {
						attrib.normals[3 * index.normal_index + 0],
						attrib.normals[3 * index.normal_index + 1],
						attrib.normals[3 * index.normal_index + 2]
					};
				else
					vertex.normal = glm::vec3(0.0f); // sera calculé plus tard

				// deduplicate vertices
				if (!uniqueVertices.contains(vertex)) {
					uniqueVertices[vertex] = static_cast<uint32_t>(modelData.vertices.size());
					modelData.vertices.push_back(vertex);
				}

				faceIndices[k] = uniqueVertices[vertex];
			}

			modelData.indices.push_back(faceIndices[0]);
			modelData.indices.push_back(faceIndices[1]);
			modelData.indices.push_back(faceIndices[2]);

			// calculer la normale si elle n'était pas fournie
			if (attrib.normals.empty()) {
				glm::vec3 p0 = modelData.vertices[faceIndices[0]].pos;
				glm::vec3 p1 = modelData.vertices[faceIndices[1]].pos;
				glm::vec3 p2 = modelData.vertices[faceIndices[2]].pos;

				glm::vec3 n = glm::normalize(glm::cross(p1 - p0, p2 - p0));

				for (auto idx : faceIndices)
					modelData.vertices[idx].normal += n;
			}
		}
	}

	// normaliser les normales si calculées
	if (attrib.normals.empty()) {
		for (auto &v : modelData.vertices)
			v.normal = glm::normalize(v.normal);
	}

	return modelData;
}
