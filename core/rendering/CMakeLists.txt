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

add_subdirectory(vulkan)