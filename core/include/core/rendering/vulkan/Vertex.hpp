//
// Created by eharquin on 12/19/25.
//

#pragma once

#include <core/rendering/vulkan/header.hpp>

#include "core/rendering/Vertex.hpp"


namespace Core::Rendering::Vulkan {
	struct Vertex {
		Rendering::Vertex vertex;

		static vk::VertexInputBindingDescription getBindingDescription() {
			return { 0, sizeof(Vertex), vk::VertexInputRate::eVertex };
		}

		static std::array<vk::VertexInputAttributeDescription, 4> getAttributeDescriptions() {
			return {
				vk::VertexInputAttributeDescription(0, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, vertex.pos)),
				vk::VertexInputAttributeDescription(1, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, vertex.color)),
				vk::VertexInputAttributeDescription(2, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, vertex.normal)),
				vk::VertexInputAttributeDescription(3, 0, vk::Format::eR32G32Sfloat, offsetof(Vertex, vertex.texCoord))
			};
		}
	};
}
