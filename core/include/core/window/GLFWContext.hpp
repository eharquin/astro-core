//
// Created by eharquin on 12/15/25.
//
// glfw_context.hpp

#pragma once

namespace Core {

	class GLFWContext {
	public:
		GLFWContext();
		~GLFWContext();

		GLFWContext(const GLFWContext&) = delete;
		GLFWContext& operator=(const GLFWContext&) = delete;
	};

} // namespace Core