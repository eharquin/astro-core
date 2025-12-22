//
// Created by eharquin on 12/21/25.
//

#pragma once

#include <core/rendering/vulkan/Context.hpp>
#include <core/common/RenderTypes.hpp>

namespace Core::Rendering::Vulkan {
	class MeshManager {
	public:
		explicit MeshManager(Context& context);

		MeshID createMesh(const MeshData& meshData);

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