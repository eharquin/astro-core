# CMake/FetchLinkGLM.CMake
include(FetchContent)

# -------------------------
# Function to fetch & link GLM
# -------------------------
function(FetchLinkGLM TARGET ACCESS)

    # Fetch
    FetchContent_Declare(
            glm
            GIT_REPOSITORY https://github.com/g-truc/glm.git
            GIT_TAG 1.0.2
    )

    FetchContent_MakeAvailable(glm)
    # Optionally, export the GLM target name for convenience
    set(GLM_TARGET glm::glm CACHE INTERNAL "GLM target name")

    # Link
    target_link_libraries(${TARGET} ${ACCESS} glm::glm)

endfunction()
