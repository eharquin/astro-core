//
// Created by eharquin on 12/17/25.
//

#include <core/utils/ImageUtils.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <cstring>
#include <stdexcept>


namespace Core::Utils {
	TextureData readTexture(const std::string &filename) {
		int texWidth, texHeight, texChannels;
		stbi_uc* pixels = stbi_load(filename.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
		if (!pixels)
			throw std::runtime_error("failed to load texture image!");

		TextureData textureData;
		textureData.width = texWidth;
		textureData.height = texHeight;
		textureData.nbChannels = texChannels;
		textureData.pixels.resize(texWidth * texHeight * 4);
		std::memcpy(textureData.pixels.data(), pixels, texWidth * texHeight * 4);
		stbi_image_free(pixels);

		return textureData;
	}
}