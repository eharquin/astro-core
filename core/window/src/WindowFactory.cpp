//
// Created by eharquin on 12/27/25.
//


#include <core/window/WindowFactory.hpp>

#include <unordered_map>

namespace Core::Window {

	static std::unordered_map<WindowsAPI,WindowContextFactory> factories;

	void registerWindowContextFactory(WindowsAPI api, WindowContextFactory factory) {
		factories[api] = std::move(factory);
	}

	std::unique_ptr<IWindowContext> createWindowContext(WindowsAPI api) {
		auto it = factories.find(api);
		if (it == factories.end())
			throw std::runtime_error("Window API not registered");
		return it->second();
	}
}