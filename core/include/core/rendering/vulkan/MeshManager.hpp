//
// Created by eharquin on 12/21/25.
//

#pragma once

#include <core/rendering/vulkan/Context.hpp>
#include <core/rendering/vulkan/Vertex.hpp>

namespace Core::Rendering::Vulkan {
	class MeshManager {
	public:
		using MeshID = uint32_t;

		explicit MeshManager(Context& context);

		MeshID createMesh(
			const std::vector<Vertex>& vertices,
			const std::vector<uint32_t>& indices
		);

		struct Mesh {
			vk::raii::Buffer vertexBuffer = nullptr;
			vk::raii::DeviceMemory vertexMemory = nullptr;
			vk::raii::Buffer indexBuffer = nullptr;
			vk::raii::DeviceMemory indexMemory = nullptr;
			uint32_t indexCount = 0;
		};

		const Mesh& get(MeshID id) const {return _meshes.at(id);}

	private:
		Context& _context;
		std::vector<Mesh> _meshes;
	};
}