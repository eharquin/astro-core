#pragma once

#include <array>
#include <tiny_obj_loader.h>

#include <unordered_map>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>
#include <glm/glm.hpp>

#include <core/rendering/Vertex.hpp>

namespace Core::Utils {

	struct ModelData {
		std::vector<Rendering::Vertex> vertices;
		std::vector<uint32_t> indices;
	};

	ModelData loadModel(const std::string &filename);
}
