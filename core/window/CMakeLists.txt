############################################
# Core Window module
############################################
add_library(core_window
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Window.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/InputState.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/GLFWContext.cpp
)

target_include_directories(core_window
        PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(core_window
        PRIVATE glfw
)