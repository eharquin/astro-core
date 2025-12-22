//
// Created by eharquin on 12/19/25.
//

#pragma once
#include <cstdint>
#include <string>

#include <core/common/RenderTypes.hpp>

namespace Core::Rendering {

	class IRenderer {
	public:
		virtual ~IRenderer() = default;

		virtual void init() = 0;
		virtual void drawFrame() = 0;
		virtual void shutdown() = 0;

		virtual MeshID createMesh(const MeshData& meshData) = 0;
		virtual TextureID createTexture(const TextureData& textureData) = 0;
		virtual void createPipeline(const std::string& name, const ShaderData& shaderData) = 0;
		virtual void addInstance(MeshID mesh, uint32_t texture = 0) = 0;
	};
}
