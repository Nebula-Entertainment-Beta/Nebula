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