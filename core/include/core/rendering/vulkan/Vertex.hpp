//
// Created by eharquin on 12/19/25.
//

#pragma once

#include <core/rendering/vulkan/Header.hpp>
#include <glm/glm.hpp>


namespace Core::Rendering::Vulkan {
	struct Vertex {
		glm::vec3 pos;
		glm::vec3 color;
		//glm::vec3 normal;
		glm::vec2 texCoord;

		static vk::VertexInputBindingDescription getBindingDescription() {
			return { 0, sizeof(Vertex), vk::VertexInputRate::eVertex };
		}

		static std::array<vk::VertexInputAttributeDescription, 3> getAttributeDescriptions() {
			return {
				vk::VertexInputAttributeDescription(0, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, pos)),
				vk::VertexInputAttributeDescription(1, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, color)),
				vk::VertexInputAttributeDescription(2, 0, vk::Format::eR32G32Sfloat, offsetof(Vertex, texCoord))
			};
		}
	};
}