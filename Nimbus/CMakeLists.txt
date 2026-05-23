cmake_minimum_required(VERSION 3.15)

# No separate project() — inherits toolchain from the repo root.

add_executable(Nimbus
    src/main.cpp
    src/nimbus.cpp
    src/player_Script.cpp
    src/mainCamera_Script.cpp
    src/register_Script.cpp
    
)

target_link_libraries(Nimbus PRIVATE Nebula)
