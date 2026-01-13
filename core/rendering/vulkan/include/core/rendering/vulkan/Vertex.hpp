//
// Created by eharquin on 12/19/25.
//

#pragma once

#include <core/rendering/vulkan/header.hpp>
#include <core/common/RenderTypes.hpp>

namespace Core::Rendering::Vulkan {
	struct Vertex {
		Core::Vertex vertex;

		static vk::VertexInputBindingDescription getBindingDescription() {
			return { 0, sizeof(Core::Vertex), vk::VertexInputRate::eVertex };
		}

		static std::array<vk::VertexInputAttributeDescription, 4> getAttributeDescriptions() {
			return {
				vk::VertexInputAttributeDescription(0, 0, vk::Format::eR32G32B32Sfloat, offsetof(Core::Vertex, vertex.pos)),
				vk::VertexInputAttributeDescription(1, 0, vk::Format::eR32G32B32Sfloat, offsetof(Core::Vertex, vertex.color)),
				vk::VertexInputAttributeDescription(2, 0, vk::Format::eR32G32B32Sfloat, offsetof(Core::Vertex, vertex.normal)),
				vk::VertexInputAttributeDescription(3, 0, vk::Format::eR32G32Sfloat, offsetof(Core::Vertex, vertex.texCoord))
			};
		}
	};
}
