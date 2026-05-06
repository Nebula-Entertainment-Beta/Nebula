/**
 * @file component.h
 * @brief Core **component** POD structs attached to `Scene` entities.
 *
 * - `TransformComponent` — wraps `Transform3D` (position, yaw, scale).
 * - `MeshRendererComponent` — placeholder mesh/material ids for a future asset pipeline.
 * - `CameraComponent` — third-person orbit parameters mirrored by `Camera3D` at runtime.
 * - `ScriptComponent` — string id looked up in `ScriptRegistry` (e.g. `"Player"`).
 */
#pragma once
#include <cstdint>
#include <string>
#include <glm/glm.hpp>
#include "transform3D.h"


namespace Nebula{

  //transform component wrapper
  struct TransformComponent{
    Transform3D transform;
    

  };

  struct MeshRendererComponent{
    uint32_t m_meshID = 0;
    uint32_t m_materialID = 0;

  };

  struct CameraComponent{
    glm::vec3 pivotOffset{0.0f, 0.35f, 0.0f};
    float distance = 6.0f;
    float yaw = 0.7f;
    float pitch = -0.3f;
    float fov = 55.0f;
    float nearClip = 0.1f;
    float farClip = 100.0f;
    bool isPrimary = true;

  };

  struct ScriptComponent{
    std::string scriptName;
  };


  
}