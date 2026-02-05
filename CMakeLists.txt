cmake_minimum_required(VERSION 3.15)

add_subdirectory(App)
add_subdirectory(Nebula)

target_link_libraries(Game PRIVATE Nebula)