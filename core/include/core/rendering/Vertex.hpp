//
// Created by eharquin on 12/19/25.
//

#pragma once
#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>
#include <functional>

namespace Core::Rendering {

	struct Vertex {
		glm::vec3 pos;
		glm::vec3 color;
		glm::vec3 normal;
		glm::vec2 texCoord;

		bool operator==(const Vertex &other) const {
			return pos == other.pos &&
				   color == other.color &&
				   normal == other.normal &&
				   texCoord == other.texCoord;
		}
	};

}

template <>
struct std::hash<Core::Rendering::Vertex> {
	size_t operator()(Core::Rendering::Vertex const &vertex) const noexcept {
		size_t h = std::hash<glm::vec3>()(vertex.pos);
		h ^= std::hash<glm::vec3>()(vertex.color) << 1;
		h ^= std::hash<glm::vec3>()(vertex.normal) << 1;
		h ^= std::hash<glm::vec2>()(vertex.texCoord) << 1;
		return h;
	}
};