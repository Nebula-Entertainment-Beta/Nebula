#pragma once
#include <memory>
#include <string>

#include "assetHandles.h"
#include "math_types.h"
#include "shader.h"

namespace Nebula
{
  class Texture;

  struct Material
  {
    std::string vertPath;
    std::string fragPath;
    std::shared_ptr<Shader> shader;
    Vec3 color{1.0f, 1.0f, 1.0f};
    std::shared_ptr<Texture> albedoTexture;
  };
}