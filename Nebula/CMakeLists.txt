cmake_minimum_required(VERSION 3.15)

project(Nebula VERSION 0.1
               LANGUAGES CXX)

add_subdirectory(vendor/glfw)
add_subdirectory(vendor/glad)
add_subdirectory(vendor/glm)

add_library(Nebula STATIC
    src/Window.cpp
    src/detail/openGL_GraphicsContext.cpp 
    src/imageDecoder.cpp 
    src/assetProvider.cpp 
    Math/camera2D.cpp
    Math/camera3D.cpp
    Math/transform2D.cpp
    Math/transform3D.cpp
    Application/application.cpp
    Application/sceneSerializer.cpp
    Application/scene.cpp
    Application/input.cpp
    Application/timestep.cpp
    Application/input_Actions.cpp
    Application/script_Registry.cpp
    Application/clock.cpp
    render_pipeline/interface/renderAPI.cpp
    render_pipeline/interface/renderer.cpp
    render_pipeline/interface/texture.cpp
    render_pipeline/openGL/openGL_Renderer.cpp
    render_pipeline/openGL/openGL_Shader.cpp
    render_pipeline/openGL/openGL_VertexBuffer.cpp
    render_pipeline/openGL/openGL_VertexArray.cpp
    render_pipeline/openGL/openGL_IndexBuffer.cpp
    render_pipeline/openGL/openGL_texture.cpp
    render_pipeline/openGL/openGL_factories.cpp


)
# Public API: headers under `include/` only (no glm in public headers; glm is PRIVATE for engine .cpp).
target_include_directories(Nebula PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_include_directories(Nebula PRIVATE
    include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/Math
    ${CMAKE_CURRENT_SOURCE_DIR}/Application
    ${CMAKE_CURRENT_SOURCE_DIR}/vendor
    ${CMAKE_CURRENT_SOURCE_DIR}/vendor/glfw/include
    ${CMAKE_CURRENT_SOURCE_DIR}/render_pipeline/interface
    ${CMAKE_CURRENT_SOURCE_DIR}/render_pipeline/openGL

)

find_package(nlohmann_json CONFIG REQUIRED)
target_link_libraries(Nebula PRIVATE glfw glad nlohmann_json::nlohmann_json glm::glm-header-only)  



