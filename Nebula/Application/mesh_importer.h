#pragma once

#include <string_view>

#include "assetProvider.h"
#include "mesh_asset.h"
#include "mesh_import_options.h"

namespace Nebula
{
  bool importMeshAsset(const IAssetProvider &assets, std::string_view logicalPath, MeshAsset &out);

  bool importMeshObj(const IAssetProvider &assets, std::string_view logicalPath, MeshAsset &out);

  bool importMeshWithOptions(const IAssetProvider &assets, const MeshImportOptions &options, MeshAsset &out);
}
