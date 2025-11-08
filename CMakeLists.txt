# CMakeLists.txt

cmake_minimum_required(VERSION 3.28...4.0)
project(astro-core LANGUAGES C CXX)

# -------------------------
# Core library
# -------------------------
add_library(${PROJECT_NAME} STATIC core/lib.cpp)
include_directories(core)
target_include_directories(${PROJECT_NAME} PUBLIC core)
set_target_properties(${PROJECT_NAME}  PROPERTIES
        CXX_STANDARD 23
        CXX_STANDARD_REQUIRED ON
)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")


# -------------------------
# Dependencies
# -------------------------

# Find Vulkan, glslc etc... 
include(FindLinkVulkan)
FindLinkVulkan(${PROJECT_NAME} PUBLIC) # Expose GLSLC_EXECUTABLE

# Fetch and Link GLM
include(FetchLinkGLM)
FetchLinkGLM(${PROJECT_NAME} PUBLIC)
# Fetch and Link GLFW
include(FetchLinkGLFW)
FetchLinkGLFW(${PROJECT_NAME} PUBLIC)
# Fetch and Link IMGUI
include(FetchLinkIMGUI)
FetchLinkIMGUI(${PROJECT_NAME} PUBLIC)

# -------------------------
# Executable
# -------------------------
add_executable(astro-app app/main.cpp)  # Your actual app entry point
# Link executable to core library
target_link_libraries(astro-app PRIVATE astro-core)

set_target_properties(astro-app  PROPERTIES
        CXX_STANDARD 23
        CXX_STANDARD_REQUIRED ON
)