cmake_minimum_required(VERSION 3.15)

# No separate project() — inherits toolchain from the repo root.

add_executable(FlappyBird
    src/main.cpp
    src/flappyBird.cpp
)

target_link_libraries(FlappyBird PRIVATE Nebula)
