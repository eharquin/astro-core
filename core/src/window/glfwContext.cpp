//
// Created by eharquin on 12/15/25.
//

// glfw_context.cpp

#include <core/window/glfwContext.hpp>

#include <GLFW/glfw3.h>
#include <iostream>


namespace Core {

	static void GLFWErrorCallback(int error, const char* description) {
		std::cerr << "GLFW Error [" << error << "] " << description << '\n';
	}

	GLFWContext::GLFWContext() {
		glfwSetErrorCallback(GLFWErrorCallback);

		if (!glfwInit())
			throw std::runtime_error("Failed to initialize GLFW");
	}

	GLFWContext::~GLFWContext() {
		glfwTerminate();
	}

} // namespace Core