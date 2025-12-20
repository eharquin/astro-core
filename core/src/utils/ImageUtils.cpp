//
// Created by eharquin on 12/17/25.
//

#include <core/utils/ImageUtils.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <cstring>
#include <stdexcept>

Core::Utils::ImageData Core::Utils::readImage(const std::string &filename) {
	int texWidth, texHeight, texChannels;
	stbi_uc* pixels = stbi_load(filename.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
	if (!pixels)
		throw std::runtime_error("failed to load texture image!");

	Core::Utils::ImageData imageData;
	imageData.width = texWidth;
	imageData.height = texHeight;
	imageData.channels = texChannels;
	imageData.pixels.resize(texWidth * texHeight * 4);
	std::memcpy(imageData.pixels.data(), pixels, texWidth * texHeight * 4);
	stbi_image_free(pixels);

	return imageData;
}
