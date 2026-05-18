#pragma once

#include <string>

#include "math_types.h"

namespace Nebula
{
  struct MaterialAsset
  {
    std::string vertPath;
    std::string fragPath;
    Vec3 albedo{1.0f, 1.0f, 1.0f};
    std::string albedoTexturePath;
  };
}
