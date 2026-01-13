add_library(core_window
        ${CMAKE_CURRENT_SOURCE_DIR}/src/WindowFactory.cpp
)

target_include_directories(core_window
        PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(core_window
        PUBLIC core_common
)

# Backends
add_subdirectory(glfw)
# add_subdirectory(sdl)   # later

