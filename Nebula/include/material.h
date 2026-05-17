#pragma once
#include <cstdint>
#include <memory>
#include "math_types.h"
#include "shader.h"

namespace Nebula
{
  using MaterialHandle = uint32_t;

  constexpr MaterialHandle kInvalidMaterial = UINT32_MAX;
  constexpr MaterialHandle kBuiltinMaterialGround = 0;
  constexpr MaterialHandle kBuiltinMaterialCube = 1;

  struct Material
  {
    std::string vertPath; // "shaders/solid_color.vert"
    std::string fragPath;
    std::shared_ptr<Shader> shader;
    Vec3 color{1.0f, 1.0f, 1.0f}; // maps to uniform uColor in your builtin shader
  };
}