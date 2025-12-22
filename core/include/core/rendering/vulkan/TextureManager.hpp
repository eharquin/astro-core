//
// Created by eharquin on 12/21/25.
//

#pragma once
#include <string>
#include <vector>

#include <core/rendering/vulkan/Context.hpp>

namespace Core::Rendering::Vulkan {
	class TextureManager {

		struct Texture {
			vk::raii::Image image = nullptr;
			vk::raii::DeviceMemory memory = nullptr;
			vk::raii::ImageView view = nullptr;
			vk::raii::Sampler sampler = nullptr;
		};

	public:
		explicit TextureManager(Context& context);

		using TextureID = uint32_t;

		TextureID loadTexture(const std::string& path);

		TextureID createDummyTexture();

		const Texture& get(TextureID id) const { return _textures.at(id); }

	private:

		Context& _context;
		std::vector<Texture> _textures;

	};
}