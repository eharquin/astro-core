//
// Created by eharquin on 12/19/25.
//

#pragma once
#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>
#include <functional>

namespace Core {

	using MeshID = uint32_t;
	using TextureID = uint32_t;

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

	struct MeshData {
		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;
	};

	struct TextureData {
		std::vector<uint8_t> pixels; // RGBA or user-defined format
		uint32_t width;
		uint32_t height;
		uint32_t nbChannels;
	};

	struct ModelData {
		MeshID meshID;
		TextureID textureID;
	};

	struct ShaderData {
		std::vector<char> code;
	};

}



template <>
struct std::hash<Core::Vertex> {
	size_t operator()(Core::Vertex const &vertex) const noexcept {
		size_t h = std::hash<glm::vec3>()(vertex.pos);
		h ^= std::hash<glm::vec3>()(vertex.color) << 1;
		h ^= std::hash<glm::vec3>()(vertex.normal) << 1;
		h ^= std::hash<glm::vec2>()(vertex.texCoord) << 1;
		return h;
	}
};