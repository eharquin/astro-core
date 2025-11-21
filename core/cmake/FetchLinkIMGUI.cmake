# cmake/FetchLinkIMGUI.cmake
include(FetchContent)

# -------------------------
# Function to fetch & link IMGUI
# -------------------------
function(FetchLinkIMGUI TARGET ACCESS)

    # Fetch
    FetchContent_Declare(
            imgui
            GIT_REPOSITORY https://github.com/ocornut/imgui.git
            GIT_TAG        v1.92.4-docking
    )

    FetchContent_MakeAvailable(imgui)
    # Optionally, export the IMGUI target name for convenience
    set(IMGUI_TARGET imgui CACHE INTERNAL "IMGUI target name")


    # If imgui doesn't provide a CMake target, create one manually
    add_library(imgui STATIC
            ${imgui_SOURCE_DIR}/imgui.cpp
            ${imgui_SOURCE_DIR}/imgui_draw.cpp
            ${imgui_SOURCE_DIR}/imgui_widgets.cpp
            ${imgui_SOURCE_DIR}/imgui_tables.cpp
            ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
            ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
    )
    target_include_directories(imgui PUBLIC
            ${imgui_SOURCE_DIR}
            ${imgui_SOURCE_DIR}/backends
    )
    target_link_libraries(${TARGET} ${ACCESS} imgui)

    # Link ImGui to GLFW (needed for imgui_impl_glfw.cpp)
    target_link_libraries(imgui PRIVATE glfw)

endfunction()
