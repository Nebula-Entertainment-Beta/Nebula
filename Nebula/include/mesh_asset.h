#pragma once

#include <cstdint>
#include <vector>

#include "math_types.h"

namespace Nebula
{
  struct MeshVertex
  {
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    float u = 0.0f;
    float v = 0.0f;
  };

  struct MeshAsset
  {
    std::vector<MeshVertex> vertices;
    std::vector<uint32_t> indices;
    Vec3 boundsMin{};
    Vec3 boundsMax{};
  };

  void computeMeshBounds(MeshAsset &mesh);
}
