cmake_minimum_required(VERSION 3.21)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 COMPONENTS Widgets OpenGLWidgets QUIET)
if(NOT Qt6_FOUND)
  find_package(Qt6 REQUIRED COMPONENTS Widgets OpenGLWidgets PATHS
    "C:/Qt/6.7.3/msvc2019_64"
    "C:/Qt/6.7.3/msvc2022_64"
  )
endif()

# Reusable editor library: engine + Qt-only editor shell.
set(NEBULA_EDITOR_LIB_SOURCES
  src/editorApplication.cpp
  src/editorLog.cpp
  src/editorTemplate.cpp
  src/editor/assetCatalog.cpp
  src/editor/editorPicking.cpp
  src/editor/editorFlyCamera.cpp
  src/qt/qtGraphicsContext.cpp
  src/qt/qtGizmoTool.cpp
  src/qt/qtMainWindow.cpp
  src/qt/qtNodeGraphPanel.cpp
  src/qt/qtSceneViewEmbed.cpp
)

add_library(NebulaEditorLib STATIC ${NEBULA_EDITOR_LIB_SOURCES})

target_include_directories(NebulaEditorLib PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/src
  ${CMAKE_CURRENT_SOURCE_DIR}/src/editor
  ${CMAKE_CURRENT_SOURCE_DIR}/src/qt
  ${CMAKE_SOURCE_DIR}/Nebula/vendor
)

target_link_libraries(NebulaEditorLib PUBLIC Nebula Qt6::Widgets Qt6::OpenGLWidgets glm::glm-header-only)
# GLFW native-access for embedding the engine window into the Qt shell.
target_link_libraries(NebulaEditorLib PRIVATE glfw)
if(WIN32)
  target_link_libraries(NebulaEditorLib PRIVATE dwmapi)
endif()

add_executable(NebulaEditor src/qt/qt_editor_main.cpp)
target_link_libraries(NebulaEditor PRIVATE NebulaEditorLib)
