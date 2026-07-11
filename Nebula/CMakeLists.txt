cmake_minimum_required(VERSION 3.15)

project(Nebula VERSION 0.1
               LANGUAGES CXX)

add_subdirectory(vendor/glfw)
add_subdirectory(vendor/glad)
add_subdirectory(vendor/glm)

add_library(Nebula STATIC
    src/Window.cpp
    src/detail/graphics_context_factory.cpp
    src/detail/openGL_GraphicsContext.cpp
    src/detail/vulkan_GraphicsContext.cpp
    src/imageDecoder.cpp 
    src/assetProvider.cpp 
    Math/camera2D.cpp
    Math/camera3D.cpp
    Math/transform2D.cpp
    Math/transform3D.cpp
    Application/application.cpp
    Application/entityComponentJson.cpp
    Application/prefabSerializer.cpp
    Application/prefabOverrides.cpp
    Application/prefabService.cpp
    Application/sceneSerializer.cpp
    Application/scene.cpp
    Application/registry.cpp
    Application/input.cpp
    Application/timestep.cpp
    Application/input_Actions.cpp
    Application/script_Registry.cpp
    Application/scriptFields.cpp
    Application/scriptParams.cpp
    Application/clock.cpp
    Application/world.cpp 
    Application/scriptSystem.cpp
    Application/systemScheduler.cpp
    Application/asset_id.cpp
    Application/asset_database.cpp
    Application/assetManager.cpp
    Application/builtinMeshes.cpp
    Application/mesh_asset.cpp
    Application/mesh_importer.cpp
    Application/mesh_upload.cpp
    Application/renderSystem.cpp
    Application/collider_fit.cpp
    Application/collider_debug.cpp
    Application/transform_gizmo.cpp
    Application/eventBus.cpp
    Application/null_physics_world.cpp
    Application/simple_physics_world.cpp
    Application/physics/collision_math.cpp
    Application/physics_system.cpp
    Application/physics_query_adapter.cpp
    Application/scene_query.cpp
    render_pipeline/interface/renderer.cpp
    render_pipeline/interface/texture.cpp
    render_pipeline/openGL/openGL_Renderer.cpp
    render_pipeline/openGL/openGL_Shader.cpp
    render_pipeline/openGL/openGL_VertexBuffer.cpp
    render_pipeline/openGL/openGL_VertexArray.cpp
    render_pipeline/openGL/openGL_IndexBuffer.cpp
    render_pipeline/openGL/openGL_texture.cpp
    render_pipeline/openGL/openGL_resource_factory.cpp
    render_pipeline/openGL/openGL_renderapiFactory.cpp
    render_pipeline/interface/render_backend_factory.cpp
    render_pipeline/vulkan/vulkan_renderapiFactory.cpp

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
    ${CMAKE_CURRENT_SOURCE_DIR}/render_pipeline/vulkan

)

find_package(nlohmann_json CONFIG REQUIRED)
target_link_libraries(Nebula PRIVATE glfw glad nlohmann_json::nlohmann_json glm::glm-header-only)
