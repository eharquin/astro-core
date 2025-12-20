//
// Created by eharquin on 12/19/25.
//

#pragma once

#include <core/rendering/Renderer.hpp>

#include <core/rendering/vulkan/Context.hpp>


namespace Core::Rendering::Vulkan {


	class Renderable : public IRenderable {
	public:
		Renderable(const Model& model);
		void draw() override;

	private:
		// Model data, buffers, etc.
		Model _model;


	};


	class Renderer : public IRenderer {
	public:
		Renderer(Context& context);

		void addRenderable(std::shared_ptr<IRenderable> obj) override;

		void addRenderable(const Model& model) override;

		void drawFrame() override;

		void resize(int width, int height) override;

	private:
		Context& _context;
		std::vector<std::shared_ptr<IRenderable>> _objects;

	};
} // Core

