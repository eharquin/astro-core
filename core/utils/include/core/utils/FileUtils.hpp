#pragma once

#include <string>

#include <core/common/RenderTypes.hpp>

namespace Core::Utils {
	ShaderData readShader(const std::string& filePath);
}