cmake_minimum_required(VERSION 3.15)

# No separate project() — inherits toolchain from the repo root.

add_library(NimbusScripts STATIC
  src/register_Script.cpp
  src/player_Script.cpp
  src/mainCamera_Script.cpp
)

target_include_directories(NimbusScripts PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(NimbusScripts PUBLIC Nebula)

add_executable(Nimbus
  src/main.cpp
  src/nimbus.cpp
)

target_link_libraries(Nimbus PRIVATE Nebula NimbusScripts)
