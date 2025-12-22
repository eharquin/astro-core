############################################
# Core Common module
############################################
add_library(core_common INTERFACE)
target_include_directories(core_common
        INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_link_libraries(core_common
        INTERFACE glm::glm
)