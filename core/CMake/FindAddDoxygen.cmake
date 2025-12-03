# CMake/FindAddDoxygen.CMake

# -------------------------
# Macro to find and add Doxygen
# -------------------------
function(FindAddDoxygen)

    # Find Doxygen documentation
    find_package(Doxygen REQUIRED)

    if (DOXYGEN_FOUND)
        message(STATUS "Doxygen found: documentation target enabled")

        set(DOXYFILE_IN ${CMAKE_CURRENT_SOURCE_DIR}/docs/Doxyfile.in)
        set(DOXYFILE_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

        # Configure the Doxyfile
        configure_file(${DOXYFILE_IN} ${DOXYFILE_OUT} @ONLY)

        # Add a custom target for docs
        add_custom_target(astro-docs
                COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYFILE_OUT}
                WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
                COMMENT "Generating API documentation with Doxygen"
                VERBATIM
        )
    else ()
        message(WARNING "Doxygen not found: documentation target disabled")
    endif ()

endfunction()
