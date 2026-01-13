//
// Created by eharquin on 12/27/25.
//

#pragma once
#include <functional>
#include <memory>

#include <core/rendering/IContext.hpp>
#include <core/rendering/GraphicsAPI.hpp>

namespace Core::Rendering {


	using GraphicsContextFactory =
		std::function<std::unique_ptr<IContext>()>;

	void registerGraphicsContextFactory(
		GraphicsAPI api,
		GraphicsContextFactory factory
	);

	std::unique_ptr<IContext> createGraphicsContext(GraphicsAPI api);
}
