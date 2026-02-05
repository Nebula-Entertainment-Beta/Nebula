cmake_minimum_required(VERSION 3.15)

add_executable(Game src/main.cpp)
target_include_directories(Game PRIVATE include)
target_link_libraries(Game PUBLIC Nebula)