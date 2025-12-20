//
// Created by eharquin on 12/19/25.
//

#pragma once

#include <memory>

#include <core/rendering/Model.hpp>

namespace Core::Rendering {

	class IRenderable {
	public:
		virtual ~IRenderable() = default;

		// Draw this object using the current renderer
		virtual void draw() = 0;
	};

	class IRenderer {
	public:
		virtual ~IRenderer() = default;

		// Add a renderable object
		virtual void addRenderable(std::shared_ptr<IRenderable> obj) = 0;
		virtual void addRenderable(const Model& model) = 0;

		// Render a frame
		virtual void drawFrame() = 0;

		// For resizing the viewport / swapchain
		virtual void resize(int width, int height) = 0;
	};

}