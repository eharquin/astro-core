//
// Created by eharquin on 12/17/25.
//

#pragma once

#include <string>
#include <vector>

namespace Core::Utils {

	struct ImageData {
		int width;
		int height;
		int channels;
		std::vector<int> pixels;
	};

	ImageData readImage(const std::string &filename);
}
