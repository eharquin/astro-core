//
// Created by eharquin on 12/27/25.
//

#pragma once

#include <core/rendering/ContextFactory.hpp>
#include <core/rendering/vulkan/Context.hpp>


namespace Core::Rendering::Vulkan {
	struct VulkanRegistrar {
		VulkanRegistrar() {
			Core::Rendering::registerGraphicsContextFactory(
				Core::Rendering::GraphicsAPI::Vulkan,
				[] {
					return std::make_unique<Context>();
				}
			);
		}
	};

	static VulkanRegistrar registrar;
}