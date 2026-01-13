//
// Created by eharquin on 12/27/25.
//

#pragma once

#include <functional>
#include <memory>

#include "IWindowContext.hpp"
#include "WindowsAPI.hpp"

namespace Core::Window {

	using WindowContextFactory =
		std::function<std::unique_ptr<IWindowContext>()>;

	void registerWindowContextFactory(
		WindowsAPI api,
		WindowContextFactory factory
	);

	std::unique_ptr<IWindowContext> createWindowContext(WindowsAPI api);
}
