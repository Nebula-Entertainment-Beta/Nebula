cmake_minimum_required(VERSION 3.15)

# No separate project() — inherits toolchain from the repo root.

add_library(NimbusScripts STATIC
  src/register_Script.cpp
  src/player_Script.cpp
  src/mainCamera_Script.cpp
  src/enemy_Script.cpp
  src/sceneDefaults.cpp
  src/waveSpawner.cpp
  src/combatSpawn.cpp
  src/combat.cpp 
  src/combatDirector.cpp
  src/combatHelper.cpp
  src/traversalDirector.cpp
  src/traversal.cpp
  src/traversalVolumes.cpp
  src/bouncePad_Script.cpp
  src/windVolume_Script.cpp
  src/checkpoint_Script.cpp
  src/goal_Script.cpp
  src/encounterDirector.cpp
)

target_include_directories(NimbusScripts PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(NimbusScripts PUBLIC Nebula)

add_executable(Nimbus
  src/main.cpp
)

target_link_libraries(Nimbus PRIVATE Nebula NimbusScripts)

# Nimbus-specific editor host (scripts + default scene). Generic editor: NebulaEditor target.
add_executable(NimbusEditor src/nimbus_editor_main.cpp)
target_include_directories(NimbusEditor PRIVATE
  ${CMAKE_SOURCE_DIR}/NebulaEditor/src
  ${CMAKE_CURRENT_SOURCE_DIR}/src
)
target_link_libraries(NimbusEditor PRIVATE NebulaEditorLib NimbusScripts)
set_target_properties(NimbusEditor PROPERTIES WIN32_EXECUTABLE FALSE)
# Ensure Qt deploy path is discoverable at runtime on Windows.
if(WIN32)
  set_property(TARGET NimbusEditor APPEND PROPERTY VS_DEBUGGER_ENVIRONMENT
    "PATH=C:/Qt/6.7.3/msvc2019_64/bin;$ENV{PATH}")
endif()
