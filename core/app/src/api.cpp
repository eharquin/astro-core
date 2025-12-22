//
// Created by eharquin on 12/22/25.
//

#include <memory>
#include <core/app/api.hpp>
#include <core/rendering/vulkan/Context.hpp>

namespace Core::App {

	std::unique_ptr<Rendering::IContext> createContext(GraphicsAPI api) {
		switch(api) {
			case GraphicsAPI::Vulkan:
				return std::make_unique<Rendering::Vulkan::Context>();
			case GraphicsAPI::OpenGL:
				// return std::make_unique<OpenGL::Context>();
				break;
			case GraphicsAPI::None:
				return nullptr;
		}
		throw std::runtime_error("Unsupported graphics API");
	}
}