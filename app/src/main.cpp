#include <iostream>
#include <stdexcept>

#include <core/app/app.hpp>

#include "helloTriangleLayer.hpp"

int main()
{
	Core::AppSpec spec;
	Core::App     app(spec);
	app.pushLayer<HelloTriangleLayer>();

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