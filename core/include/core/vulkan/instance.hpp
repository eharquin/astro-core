#pragma once

#if defined(__INTELLISENSE__) || !defined(USE_CPP20_MODULES)
#include <vulkan/vulkan_raii.hpp>
#else
import vulkan_hpp;
#endif


namespace Core::Vulkan
{
class Instance
{
public:
	Instance() = default;

	void create();

private:
	bool checkGLFWExtensionSupport(const std::vector<const char *> &requiredExtensions) const;

	//
	// bool checkValidationLayerSupport();
	//
	static std::vector<const char *> getRequiredExtensions();

	//
	// std::vector<VkExtensionProperties> getInstanceExtensions();

	vk::raii::Context  _context;
	vk::raii::Instance _instance = nullptr;
};
} // namespace Core::Vulkan
