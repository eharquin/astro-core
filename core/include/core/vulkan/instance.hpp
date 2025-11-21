#pragma once

#include <vector>

#include <vulkan/vulkan.hpp>


namespace Core {
namespace Vulkan {

    class Instance {
    public:
        Instance();
        ~Instance();

    private:
        void create();
        void destroy();

        std::vector<const char*> getGLFWExtensions();
        bool checkExtensions(std::vector<const char*> requiredExtensions);
        std::vector<VkExtensionProperties> getInstanceExtensions();
        VkInstance _instance;
    };

} // namespace Vulkan
} // namespace Core
