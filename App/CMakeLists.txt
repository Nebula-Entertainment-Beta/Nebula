cmake_minimum_required(VERSION 3.15)

add_executable(Game src/main.cpp)
target_link_libraries(Game PUBLIC Nebula)