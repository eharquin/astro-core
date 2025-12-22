############################################
# Core Rendering module interface
############################################
add_library(core_rendering INTERFACE)

target_include_directories(core_rendering
        INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Depends on core_window because Renderer interfaces reference Window
target_link_libraries(core_rendering
        INTERFACE core_common
        INTERFACE core_window
)

############################################
# Core Rendering Vulkan module
############################################
add_library(core_rendering_vulkan
        ${CMAKE_CURRENT_SOURCE_DIR}/src/vulkan/Context.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/vulkan/Renderer.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/vulkan/MeshManager.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/vulkan/PipelineManager.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/vulkan/Swapchain.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/vulkan/TextureManager.cpp
)

if (ENABLE_CPP20_MODULE)
    target_compile_definitions(core_rendering_vulkan PRIVATE USE_CPP20_MODULES=1)
endif ()

target_include_directories(core_rendering_vulkan
        PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
        PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include/vulkan
)

target_link_libraries(core_rendering_vulkan
        PRIVATE core_rendering  # links header-only interface
        PRIVATE Vulkan::cppm
)