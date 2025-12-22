//
// Created by eharquin on 12/13/25.
//
#include <core/utils/FileUtils.hpp>

#include <fstream>

namespace Core::Utils {
	ShaderData readShader(const std::string& filePath) {
		std::ifstream file(filePath, std::ios::ate | std::ios::binary);

		if (!file.is_open())
			throw std::runtime_error("failed to open file : " + filePath);

		std::vector<char> buffer(file.tellg());

		file.seekg(0, std::ios::beg);
		file.read(buffer.data(), static_cast<std::streamsize>(buffer.size()));
		file.close();

		ShaderData shaderCode{};
		shaderCode.code = std::move(buffer);
		return shaderCode;
	}
}