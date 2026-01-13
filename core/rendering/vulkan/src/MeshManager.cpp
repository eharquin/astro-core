//
// Created by eharquin on 12/21/25.
//


#include <core/rendering/vulkan/MeshManager.hpp>

namespace Core::Rendering::Vulkan {
	MeshManager::MeshManager(Context& context)
		: _context(context) {}

	MeshID MeshManager::createMesh(const MeshData& meshData) {

		Mesh mesh{};

		_context.createDeviceLocalBuffer(meshData.vertices, vk::BufferUsageFlagBits::eVertexBuffer, mesh.vertexBuffer, mesh.vertexMemory);
		_context.createDeviceLocalBuffer(meshData.indices, vk::BufferUsageFlagBits::eIndexBuffer, mesh.indexBuffer, mesh.indexMemory);

		mesh.indexCount = static_cast<uint32_t>(meshData.indices.size());
		_meshes.push_back(std::move(mesh));

		return static_cast<MeshID>(_meshes.size() - 1);
	}
}
