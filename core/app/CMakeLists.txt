############################################
# Core App module
############################################
add_library(core_app
        ${CMAKE_CURRENT_SOURCE_DIR}/src/App.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/api.cpp
)

target_include_directories(core_app
        PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(core_app
        PUBLIC core_window      # depends on window
        PUBLIC core_rendering   # depends on rendering interfaces
        PRIVATE core_rendering_vulkan  # depends on Vulkan implementation
)
