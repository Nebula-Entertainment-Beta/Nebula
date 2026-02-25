cmake_minimum_required(VERSION 3.15)

project(Game VERSION 0.1
               LANGUAGES CXX)

add_executable(Game src/main.cpp)
target_link_libraries(Game PUBLIC Nebula)
