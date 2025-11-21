# -------------------------
# App
# -------------------------

file(GLOB_RECURSE APP_SOURCES src/*.cpp)

add_executable(astro-app ${APP_SOURCES})
target_link_libraries(astro-app PRIVATE astro-core)
target_include_directories(astro-app PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
