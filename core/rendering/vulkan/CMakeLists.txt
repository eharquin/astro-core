############################################
# Core Rendering Vulkan module
############################################
add_library(core_rendering_vulkan
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Context.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Renderer.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/MeshManager.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/PipelineManager.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Swapchain.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/TextureManager.cpp
)

if (ENABLE_CPP20_MODULE)
    target_compile_definitions(core_rendering_vulkan PRIVATE USE_CPP20_MODULES=1)
endif ()

target_include_directories(core_rendering_vulkan
        PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(core_rendering_vulkan
        PRIVATE core_rendering  # links header-only interface
        PRIVATE Vulkan::cppm
)