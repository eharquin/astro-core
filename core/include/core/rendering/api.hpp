//
// Created by eharquin on 12/21/25.
//

#pragma once
#include "IContext.hpp"
#include "vulkan/Context.hpp"

namespace Core::Rendering {

	enum class GraphicsAPI {
		Vulkan,
		OpenGL,
		None
	};

	inline std::unique_ptr<IContext> createContext(Rendering::GraphicsAPI api) {
		switch(api) {
			case GraphicsAPI::Vulkan:
				return std::make_unique<Vulkan::Context>();
			case GraphicsAPI::OpenGL:
				// return std::make_unique<OpenGL::Context>();
				break;
			case GraphicsAPI::None:
				return nullptr;
		}
		throw std::runtime_error("Unsupported graphics API");
	}
}
