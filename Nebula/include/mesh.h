#pragma once
#include <cstdint>
#include <memory>
#include "vertex_array.h"

namespace Nebula
{
  using MeshHandle = uint32_t;

  constexpr MeshHandle kInvalidMesh = UINT32_MAX;
  constexpr MeshHandle kBuiltinMeshCube = 0;
  constexpr MeshHandle kBuiltinMeshGround = 1;

  struct Mesh
  {
    std::shared_ptr<VertexArray> vao;
    uint32_t indexCount = 0;
  };
}