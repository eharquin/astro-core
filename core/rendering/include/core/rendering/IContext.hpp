//
// Created by eharquin on 12/21/25.
//

#pragma once

#include <core/window/IWindow.hpp>
#include <core/window/IWindowContext.hpp>
#include <core/rendering/IRenderer.hpp>
#include <memory>

namespace Core::Rendering {

	class IContext {
	public:
		virtual ~IContext() = default;

		virtual void init(Window::IWindowContext &windowContext, const Window::IWindow& window) = 0;
		virtual void shutdown() = 0;

		virtual std::unique_ptr<IRenderer> createRenderer(const Window::IWindow &window) = 0;
	};
}