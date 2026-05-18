#pragma once
#include <cstdint>
#include <memory>
#include "vertex_array.h"
#include "assetHandles.h"

namespace Nebula
{
  struct Mesh
  {
    std::shared_ptr<VertexArray> vao;
    uint32_t indexCount = 0;
  };
}