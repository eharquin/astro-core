# CMake/FetchLinkGLFW.CMake
include(FetchContent)

# -------------------------
# Function to fetch & link GLFW
# -------------------------
function(FetchLinkGLFW TARGET ACCESS)

    # Fetch
    FetchContent_Declare(
            glfw
            GIT_REPOSITORY https://github.com/glfw/glfw.git
            GIT_TAG 3.4
    )

    FetchContent_MakeAvailable(glfw)
    # Optionally, export the GLFW target name for convenience
    set(GLFW_TARGET glfw CACHE INTERNAL "GLFW target name")

    # Link
    target_link_libraries(${TARGET} ${ACCESS} glfw)
endfunction()
