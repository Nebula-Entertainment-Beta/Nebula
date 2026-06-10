cmake_minimum_required(VERSION 3.15)

# No separate project() — inherits toolchain from the repo root.

add_library(NimbusScripts STATIC
  src/register_Script.cpp
  src/player_Script.cpp
  src/mainCamera_Script.cpp
  src/enemy_Script.cpp
  src/sceneDefaults.cpp
  src/combat.cpp 
  src/combatDirector.cpp
  src/combatHelper.cpp
)

target_include_directories(NimbusScripts PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(NimbusScripts PUBLIC Nebula)

add_executable(Nimbus
  src/main.cpp
  src/nimbus.cpp
  
)

target_link_libraries(Nimbus PRIVATE Nebula NimbusScripts)

# Nimbus-specific editor host (scripts + default scene). Generic editor: NebulaEditor target.
add_executable(NimbusEditor src/nimbus_editor_main.cpp)
target_include_directories(NimbusEditor PRIVATE
  ${CMAKE_SOURCE_DIR}/NebulaEditor/src
  ${CMAKE_CURRENT_SOURCE_DIR}/src
)
target_link_libraries(NimbusEditor PRIVATE NebulaEditorLib NimbusScripts)
target_compile_definitions(NimbusEditor PRIVATE IMGUI_IMPL_OPENGL_LOADER_GLAD)
