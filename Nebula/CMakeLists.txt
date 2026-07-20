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
    Application/scriptSystem.cpp
    Application/systemScheduler.cpp
    Application/asset_id.cpp
    Application/asset_database.cpp
    Application/assetManager.cpp
    Application/builtinMeshes.cpp
    Application/mesh_asset.cpp
    Application/mesh_import_options.cpp
    Application/mesh_importer.cpp
    Application/mesh_upload.cpp
    Application/renderSystem.cpp
    Application/collider_fit.cpp
    Application/collider_debug.cpp
    Application/transform_gizmo.cpp
    Application/debug_line_renderer.cpp
    Application/eventBus.cpp
    Application/null_physics_world.cpp
    Application/physx_physics_world.cpp
    Application/physics/collision_math.cpp
    Application/physics_system.cpp
    Application/physics_query_adapter.cpp
    Application/scene_query.cpp
    Application/fmod_audio_service.cpp
    Application/nodeGraphRuntime.cpp
    Application/environment_render.cpp
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

if(NOT TARGET nlohmann_json::nlohmann_json)
  find_package(nlohmann_json CONFIG QUIET)
  if(NOT nlohmann_json_FOUND)
    include(FetchContent)
    FetchContent_Declare(
      nlohmann_json
      GIT_REPOSITORY https://github.com/nlohmann/json.git
      GIT_TAG v3.11.3
    )
    FetchContent_MakeAvailable(nlohmann_json)
  endif()
endif()

if(NOT TARGET assimp::assimp)
  find_package(assimp QUIET)
  if(NOT assimp_FOUND)
    include(FetchContent)
    FetchContent_Declare(
      assimp
      GIT_REPOSITORY https://github.com/assimp/assimp.git
      GIT_TAG v5.4.3
    )
    set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(ASSIMP_INSTALL OFF CACHE BOOL "" FORCE)
    set(ASSIMP_BUILD_ASSIMP_TOOLS OFF CACHE BOOL "" FORCE)
    set(ASSIMP_BUILD_SAMPLES OFF CACHE BOOL "" FORCE)
    set(ASSIMP_WARNINGS_AS_ERRORS OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(assimp)
  endif()
endif()

find_package(unofficial-omniverse-physx-sdk CONFIG QUIET)
if(NOT unofficial-omniverse-physx-sdk_FOUND)
  find_package(PhysX QUIET)
endif()

target_link_libraries(Nebula PRIVATE glfw glad nlohmann_json::nlohmann_json glm::glm-header-only assimp::assimp)

if(TARGET unofficial::omniverse-physx-sdk::sdk)
  target_link_libraries(Nebula PRIVATE unofficial::omniverse-physx-sdk::sdk)
  target_compile_definitions(Nebula PRIVATE NEBULA_USE_PHYSX=1)
elseif(TARGET PhysX::PhysX)
  target_link_libraries(Nebula PRIVATE PhysX::PhysX PhysX::Foundation PhysX::Common PhysX::Extensions)
  target_compile_definitions(Nebula PRIVATE NEBULA_USE_PHYSX=1)
else()
  message(FATAL_ERROR "PhysX SDK not found. Install via vcpkg: vcpkg install physx:x64-windows and pass -DCMAKE_TOOLCHAIN_FILE=...")
endif()

# Optional FMOD Core (set FMOD_ROOT to the API folder with inc/ and lib/).
if(DEFINED ENV{FMOD_ROOT} AND NOT FMOD_ROOT)
  set(FMOD_ROOT "$ENV{FMOD_ROOT}")
endif()
if(FMOD_ROOT)
  find_path(FMOD_INCLUDE_DIR fmod.hpp PATHS "${FMOD_ROOT}/inc" "${FMOD_ROOT}/api/core/inc" NO_DEFAULT_PATH)
  find_library(FMOD_LIBRARY NAMES fmod_vc fmod PATHS "${FMOD_ROOT}/lib/x64" "${FMOD_ROOT}/api/core/lib/x64" NO_DEFAULT_PATH)
  if(FMOD_INCLUDE_DIR AND FMOD_LIBRARY)
    target_include_directories(Nebula PRIVATE "${FMOD_INCLUDE_DIR}")
    target_link_libraries(Nebula PRIVATE "${FMOD_LIBRARY}")
    target_compile_definitions(Nebula PRIVATE NEBULA_USE_FMOD=1)
    message(STATUS "FMOD found: ${FMOD_LIBRARY}")
  else()
    message(STATUS "FMOD_ROOT set but headers/libs not found; using audio stub")
  endif()
else()
  message(STATUS "FMOD not configured (optional); using audio stub — see docs/MIDDLEWARE_SETUP.md")
endif()
