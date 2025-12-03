# CMake/FindLinkVulkan.CMake

# -----------------------------------------------------------
# Function to find and link Vulkan SDK + glslc automatically
# ----------------------------------------------------------
function(FindLinkVulkan TARGET ACCESS)
    # Find Vulkan SDK
    find_package(Vulkan REQUIRED)

    if (Vulkan_FOUND)
        message(STATUS "[Vulkan] Include dir: ${Vulkan_INCLUDE_DIRS}")
        message(STATUS "[Vulkan] Library: ${Vulkan_LIBRARIES}")
    else ()
        message(FATAL_ERROR "[Vulkan] Vulkan SDK not found. Please install it.")
    endif ()

    # Find glslc compiler (optional)
    find_program(GLSLC_EXECUTABLE NAMES glslc HINTS
            "$ENV{VULKAN_SDK}/Bin"
            "$ENV{VULKAN_SDK}/bin"
    )

    if (GLSLC_EXECUTABLE)
        message(STATUS "[Vulkan] Found glslc: ${GLSLC_EXECUTABLE}")
    else ()
        message(WARNING "[Vulkan] glslc not found; shader compilation may fail.")
    endif ()

    # Link Vulkan include dirs and libs
    target_include_directories(${TARGET} ${ACCESS} ${Vulkan_INCLUDE_DIRS})
    target_link_libraries(${TARGET} ${ACCESS} ${Vulkan_LIBRARIES})

    # Expose glslc path to parent scope
    set(GLSLC_EXECUTABLE ${GLSLC_EXECUTABLE} PARENT_SCOPE)
endfunction()
