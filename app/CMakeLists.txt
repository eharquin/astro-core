# -------------------------
# App
# -------------------------
cmake_minimum_required(VERSION 3.29)
project(astroApp)

file(GLOB_RECURSE APP_SOURCES src/*.cpp)

add_executable(${PROJECT_NAME} ${APP_SOURCES})
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 20)
target_link_libraries(${PROJECT_NAME} PRIVATE astroCore)
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
