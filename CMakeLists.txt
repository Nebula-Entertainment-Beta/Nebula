cmake_minimum_required(VERSION 3.15)

project(Nebula VERSION 0.1
               LANGUAGES CXX)

add_subdirectory(App)
add_subdirectory(Nebula)
add_subdirectory(glfw)
set(CMAKE_CXX_STANDARD 17)
