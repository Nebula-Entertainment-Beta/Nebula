#pragma once

#include <string>
#include <string_view>
#include <unordered_map>

#include "asset_id.h"
#include "assetProvider.h"
#include "material_asset.h"
#include "mesh_asset.h"

namespace Nebula
{
  class AssetDatabase
  {
  public:
    explicit AssetDatabase(const IAssetProvider &assets);

    AssetGuid loadOrGetMesh(std::string_view logicalPath);
    AssetGuid loadOrGetMaterial(std::string_view logicalPath);

    const MeshAsset *getMeshAsset(AssetGuid guid) const;
    const MaterialAsset *getMaterialAsset(AssetGuid guid) const;

    std::string_view pathForGuid(AssetGuid guid) const;
    AssetGuid guidForPath(std::string_view logicalPath) const;

    void registerMeshAsset(std::string_view logicalPath, MeshAsset asset);
    void registerMaterialAsset(std::string_view logicalPath, MaterialAsset asset);

  private:
    const IAssetProvider &m_assets;
    std::unordered_map<std::string, AssetGuid> m_pathToGuid;
    std::unordered_map<AssetGuid, std::string> m_guidToPath;
    std::unordered_map<AssetGuid, MeshAsset> m_meshes;
    std::unordered_map<AssetGuid, MaterialAsset> m_materials;
    AssetGuid m_nextGuid = 1;
  };
}
