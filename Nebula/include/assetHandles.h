#pragma once
#include <cstdint>

namespace Nebula
{
  using MeshHandle = uint32_t;
  using MaterialHandle = uint32_t;

  constexpr MeshHandle kInvalidMesh = UINT32_MAX;
  constexpr MaterialHandle kInvalidMaterial = UINT32_MAX;
}
