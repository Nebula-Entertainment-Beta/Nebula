cmake_minimum_required(VERSION 3.15)

# No separate project() — inherits toolchain from the repo root.

add_executable(Nimbus
    src/main.cpp
    src/nimbus.cpp
)

target_link_libraries(Nimbus PRIVATE Nebula)
