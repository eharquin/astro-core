//
// Created by eharquin on 12/21/25.
//

#pragma once
#include <memory>
#include <core/rendering/IContext.hpp>

namespace Core::App {

	enum class GraphicsAPI {
		Vulkan,
		OpenGL,
		None
	};

	std::unique_ptr<Rendering::IContext> createContext(GraphicsAPI api);
}