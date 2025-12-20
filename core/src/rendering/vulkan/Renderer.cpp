//
// Created by eharquin on 12/19/25.
//

#include <core/rendering/vulkan/Renderer.hpp>

namespace Core::Rendering::Vulkan {
	Renderable::Renderable(const Model &model) {

	}

	void Renderable::draw() {

	}

	Renderer::Renderer(Context &context): _context(context) {}

	void Renderer::addRenderable(std::shared_ptr<IRenderable> obj) {
		_objects.push_back(obj);
	}

	void Renderer::addRenderable(const Model &model) {
		auto r = std::make_shared<Renderable>(model);
		_objects.push_back(std::make_shared<Renderable>(model));
	}

	void Renderer::drawFrame() {
		// TODO _context.beginFrame();

		for (auto& obj : _objects) {
			obj->draw();  // Each object knows how to draw itself
		}

		// TODO _context.endFrame();
	}

	void Renderer::resize(int width, int height) {
		_context.shouldRecreateSwapChain();
	}
} // Core