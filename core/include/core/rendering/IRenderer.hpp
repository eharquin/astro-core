//
// Created by eharquin on 12/19/25.
//

#pragma once
#include <cstdint>
#include <string>

#include "Vertex.hpp"

namespace Core::Rendering {

	class IRenderer {
	public:
		virtual ~IRenderer() = default;

		virtual void init() = 0;
		virtual void drawFrame() = 0;
		virtual void shutdown() = 0;

		using MeshID = uint32_t;
		using TextureID = uint32_t;

		virtual MeshID createMesh(const std::vector<Vertex> &vertices,
		                          const std::vector<uint32_t> &indices) = 0;

		virtual TextureID createTexture(const std::string& texturePath) = 0;

		virtual void addInstance(MeshID mesh, uint32_t texture = 0) = 0;
	};
}
