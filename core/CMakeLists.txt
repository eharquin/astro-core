# -------------------------
# Core library
# -------------------------
cmake_minimum_required(VERSION 3.29)

project(astroCore)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/CMake")

# Add option to enable/disable C++ 20 module
option(ENABLE_CPP20_MODULE "Enable C++ 20 module support for Vulkan" ON)

# Enable C++ module dependency scanning only if C++ 20 module is enabled
if (ENABLE_CPP20_MODULE)
    set(CMAKE_CXX_SCAN_FOR_MODULES ON)
endif ()

find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)
find_package(tinyobjloader REQUIRED)

find_package(Vulkan REQUIRED)

if (Vulkan_FOUND)
    message(STATUS "Vulkan library found: ${Vulkan_LIBRARY}")
    message(STATUS "Vulkan headers: ${Vulkan_INCLUDE_DIRS}")
    if (DEFINED Vulkan_VERSION)
        message(STATUS "Vulkan version: ${Vulkan_VERSION}")
    else()
        # fallback if Vulkan_VERSION is not set by the module
        include(VulkanHelpers) # optional if you have a helper module
        message(STATUS "Vulkan version info not available from CMake module")
    endif()
else()
    message(FATAL_ERROR "Vulkan not found!")
endif()

include(EnableVulkanModule)
enable_vulkan_module(ENABLE_CPP20_MODULE) # Expose Vulkan::cppm, use ENABLE_CPP20_MODULE

file(GLOB_RECURSE CORE_SOURCES src/*.cpp)
file(GLOB_RECURSE CORE_HEADERS include/*.hpp)

add_library(${PROJECT_NAME} STATIC ${CORE_SOURCES} ${CORE_HEADERS})
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 20)
target_link_libraries(${PROJECT_NAME}
        PRIVATE Vulkan::cppm
        PRIVATE glfw
        PRIVATE tinyobjloader)


if (ENABLE_CPP20_MODULE)
    target_compile_definitions(${PROJECT_NAME} PRIVATE USE_CPP20_MODULES=1)
endif ()