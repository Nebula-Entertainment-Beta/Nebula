#pragma once

#include <cstdint>
#include <string_view>

namespace Nebula
{
  using AssetGuid = uint64_t;

  constexpr AssetGuid kInvalidAsset = 0;

  /** Stable id from a normalized logical asset path (FNV-1a). */
  AssetGuid assetGuidFromPath(std::string_view logicalPath);
}
