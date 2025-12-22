//
// Created by eharquin on 12/21/25.
//

#include <core/rendering/vulkan/TextureManager.hpp>

namespace Core::Rendering::Vulkan {
	TextureManager::TextureManager(Context& context)
		: _context(context) {
		createDummyTexture();
	}

	TextureID TextureManager::loadTexture(const TextureData& textureData) {

		Texture texture;

		_context.createTextureImageFromData(textureData.pixels.data(), textureData.width, textureData.height, texture.image, texture.memory);

		texture.view = _context.createImageView(texture.image, vk::Format::eR8G8B8A8Srgb, vk::ImageAspectFlagBits::eColor);

		texture.sampler = _context.createTextureSampler();

		_textures.push_back(std::move(texture));

		return static_cast<TextureID>(_textures.size() - 1);
	}


	TextureID TextureManager::createDummyTexture() {
		assert(_textures.empty() && "Dummy texture must be created first!");

		Texture texture;

		// 1x1 white pixel RGBA
		uint8_t pixel[4] = { 255, 255, 255, 255 };
		uint32_t width = 1;
		uint32_t height = 1;

		// Create Vulkan image and allocate memory
		_context.createTextureImageFromData(pixel, width, height, texture.image, texture.memory);

		// Create image view
		texture.view = _context.createImageView(texture.image, vk::Format::eR8G8B8A8Srgb, vk::ImageAspectFlagBits::eColor);

		// Create sampler
		texture.sampler = _context.createTextureSampler();

		// Add to texture vector
		_textures.push_back(std::move(texture));

		// Return the index
		return static_cast<TextureID>(_textures.size() - 1);
	}
}
