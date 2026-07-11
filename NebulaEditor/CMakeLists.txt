set(IMGUI_DIR ${CMAKE_SOURCE_DIR}/Nebula/vendor/imgui)

add_library(imgui STATIC
  ${IMGUI_DIR}/imgui.cpp
  ${IMGUI_DIR}/imgui_draw.cpp
  ${IMGUI_DIR}/imgui_tables.cpp
  ${IMGUI_DIR}/imgui_widgets.cpp
  ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
  ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
)

target_include_directories(imgui PUBLIC
  ${IMGUI_DIR}
  ${IMGUI_DIR}/backends
)

target_link_libraries(imgui PUBLIC glfw glad)
target_compile_definitions(imgui PUBLIC IMGUI_IMPL_OPENGL_LOADER_GLAD)

# Reusable editor library: Nebula only (any game links this + supplies main/callbacks).
add_library(NebulaEditorLib STATIC
  src/editorApplication.cpp
  src/imGuiLayer.cpp
  src/editorLog.cpp
  src/editorTemplate.cpp
  src/panels/sceneViewPanel.cpp
  src/panels/hierarchyPanel.cpp
  src/panels/inspectorPanel.cpp
  src/panels/consolePanel.cpp
  src/panels/debugPanel.cpp
  src/render/sceneViewFrameBuffer.cpp
)

target_include_directories(NebulaEditorLib PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/src
  ${CMAKE_CURRENT_SOURCE_DIR}/src/panels
)

target_link_libraries(NebulaEditorLib PUBLIC Nebula imgui)
target_compile_definitions(NebulaEditorLib PUBLIC IMGUI_IMPL_OPENGL_LOADER_GLAD)

# Generic editor binary (no game project). For gameplay content use a game-specific target.
add_executable(NebulaEditor src/main.cpp)
target_link_libraries(NebulaEditor PRIVATE NebulaEditorLib)
target_compile_definitions(NebulaEditor PRIVATE IMGUI_IMPL_OPENGL_LOADER_GLAD)
