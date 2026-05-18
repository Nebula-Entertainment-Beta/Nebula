#pragma once

#include <string_view>

#include "assetProvider.h"
#include "mesh_asset.h"

namespace Nebula
{
  /** Imports `.obj` geometry or a `.mesh` JSON descriptor that points at an OBJ source. */
  bool importMeshAsset(const IAssetProvider &assets, std::string_view logicalPath, MeshAsset &out);

  bool importMeshObj(const IAssetProvider &assets, std::string_view logicalPath, MeshAsset &out);
}
