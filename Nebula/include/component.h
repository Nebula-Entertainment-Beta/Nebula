/**
 * @file component.h
 * @brief Core **component** POD structs attached to scene entities (`Registry` storage).
 *
 * - `TransformComponent` — wraps `Transform3D` (position, yaw, scale).
 * - `MeshRendererComponent` — placeholder mesh/material ids for a future asset pipeline.
 * - `CameraComponent` — third-person orbit parameters mirrored by `Camera3D` at runtime.
 * - `ScriptComponent` — string id looked up in `ScriptRegistry` (e.g. `"Player"`).
 */
#pragma once
#include <cstdint>
#include <string>
#include "math_types.h"
#include "transform3D.h"
#include "assetHandles.h"
#include "ecs/entity.h"

namespace Nebula
{

  // transform component wrapper
  struct TransformComponent
  {
    Transform3D transform;
  };

  struct CameraComponent
  {
    Vec3 pivotOffset{0.0f, 0.35f, 0.0f};
    float distance = 6.0f;
    float yaw = 0.7f;
    float pitch = -0.3f;
    float fov = 55.0f;
    float nearClip = 0.1f;
    float farClip = 100.0f;
    bool isPrimary = true;
    Entity targetEntity{};
    std::string targetTag;
  };

  struct followTargetComponent
  {
    Entity targetEntity{};
    std::string targetTag;
  };

  struct ScriptComponent
  {
    std::string scriptName;
    std::string paramsJson = "{}";
  };

  struct MeshRendererComponent
  {
    std::string m_meshPath;
    std::string m_materialPath;
    MeshHandle m_meshID = kInvalidMesh;
    MaterialHandle m_materialID = kInvalidMaterial;
  };

  /** Stylized sky-world atmosphere (serialized; drives clear color / fog / light). */
  struct EnvironmentComponent
  {
    Vec3 skyTop{0.35f, 0.55f, 0.95f};
    Vec3 skyBottom{0.85f, 0.9f, 1.0f};
    Vec3 fogColor{0.7f, 0.8f, 0.95f};
    float fogDensity = 0.02f;
    Vec3 lightDirection{0.4f, 0.8f, 0.3f};
    Vec3 lightColor{1.0f, 0.95f, 0.85f};
    float lightIntensity = 1.1f;
    int timeOfDayPreset = 0; // 0 day, 1 dusk, 2 night
    float cloudLayerHeight = 12.f;
    float cloudOpacity = 0.35f;
  };

}
