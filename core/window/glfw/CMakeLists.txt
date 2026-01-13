add_library(core_window_glfw
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Window.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/WindowContext.cpp
)

target_include_directories(core_window_glfw
        PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(core_window_glfw
        PUBLIC core_window
        PRIVATE glfw
)