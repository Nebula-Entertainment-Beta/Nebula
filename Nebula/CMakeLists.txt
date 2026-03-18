cmake_minimum_required(VERSION 3.15)

project(Nebula VERSION 0.1
               LANGUAGES CXX)

add_subdirectory(vendor/glfw)
add_subdirectory(vendor/glad)
include_directories(/usr/local/include)
add_library(Nebula SHARED src/Window.cpp)
target_include_directories(Nebula PUBLIC include ${CMAKE_CURRENT_SOURCE_DIR}/vendor/glfw/include)
target_link_libraries(Nebula PRIVATE glfw glad)
