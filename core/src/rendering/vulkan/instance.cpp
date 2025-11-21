#include <core/vulkan/instance.hpp>

#include <iostream>

#include <GLFW/glfw3.h>

namespace Core {
namespace Vulkan {

    Instance::Instance() {
        create();
    }

    Instance::~Instance() {
        destroy();
    }

    void Instance::destroy() {
        vkDestroyInstance(_instance, nullptr);
        _instance = nullptr;
    }

    void Instance::create() {
        VkApplicationInfo appInfo{};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "astro-engine";
        appInfo.pNext = nullptr; // optional
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "AstroEngine";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_3; // tuto used 1_0

        // get required extensions
        std::vector<const char*> requiredExtensions;
        requiredExtensions.emplace_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
        auto glfwExtensions = getGLFWExtensions();
        requiredExtensions.insert(requiredExtensions.end(), glfwExtensions.begin(), glfwExtensions.end());

        if(!checkExtensions(requiredExtensions))
            throw std::runtime_error("extensions unvailable");


        VkInstanceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pNext = nullptr; // optional
        createInfo.pApplicationInfo = &appInfo;
        createInfo.enabledExtensionCount =static_cast<uint32_t>(requiredExtensions.size());
        createInfo.ppEnabledExtensionNames = requiredExtensions.data();
        createInfo.enabledLayerCount = 0;
        createInfo.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;

        if (vkCreateInstance(&createInfo, nullptr, &_instance) != VK_SUCCESS)
            throw std::runtime_error("failed to create instance!");
    }


    std::vector<const char*> Instance::getGLFWExtensions() {
        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensionsNames = nullptr;
        glfwExtensionsNames = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        // initialize vector using input iterator
        std::vector<const char*> extensions(glfwExtensionsNames, glfwExtensionsNames + glfwExtensionCount);
        // if (enableValidationLayers)
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

        return extensions;
    }

    std::vector<VkExtensionProperties> Instance::getInstanceExtensions() {
        uint32_t extensionCount = 0;
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
        std::vector<VkExtensionProperties> extensions(extensionCount);
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

        return extensions;
    }

    bool Instance::checkExtensions(std::vector<const char*> requiredExtensions) {
        // get available instance extensions
        const auto instanceExtensions = getInstanceExtensions();

        // Check each required extension
        for (const char* required : requiredExtensions) {
            bool found = false;
            for (const auto& ext : instanceExtensions) {
                if (std::string(ext.extensionName) == required) {
                    found = true;
                    break;
                }
            }

            if (!found) {
                std::cerr << "Missing required extension: " << required << '\n';
                return false; // fail early if any extension is missing
            }
        }

        std::cout << "all required extensions have been found!" << std::endl;
        return true;
    }



} // namespace Vulkan
} // namespace Core
