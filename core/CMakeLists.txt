# -------------------------
# Core library
# -------------------------

file(GLOB_RECURSE CORE_SOURCES src/*.cpp)
file(GLOB_RECURSE CORE_HEADERS include/*.hpp)

add_library(astro-core STATIC ${CORE_SOURCES} ${CORE_HEADERS})
target_include_directories(astro-core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

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

# Find and Add Doxygen
include(FindAddDoxygen)
FindAddDoxygen()