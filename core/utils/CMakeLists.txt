############################################
# Core Utils module
############################################
add_library(core_utils
        ${CMAKE_CURRENT_SOURCE_DIR}/src/FileUtils.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/ImageUtils.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/MeshUtils.cpp
)

target_include_directories(core_utils
        PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# tinyobjloader is needed for ModelUtils.cpp
target_link_libraries(core_utils
        PUBLIC core_common
        PRIVATE tinyobjloader
)