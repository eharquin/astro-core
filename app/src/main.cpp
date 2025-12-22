#include <iostream>
#include <stdexcept>

#include <core/app/App.hpp>

#include "SimpleModelLayer.hpp"

using namespace Core::App;

int main()
{
	AppSpec spec;
	App     app(spec);
	app.pushLayer<SimpleModelLayer>();

	try
	{
		app.run();
	}
	catch (const std::exception &e)
	{
		std::cerr << e.what() << std::endl;
		return 1;
	}

	return 0;
}