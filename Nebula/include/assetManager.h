#pragma once

#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "asset_database.h"
#include "asset_id.h"
#include "mesh.h"
#include "material.h"
#include "mesh_asset.h"
#include "assetProvider.h"
#include "renderResources.h"
#include "scene.h"

namespace Nebula
{
  class AssetManager
  {
  public:
    explicit AssetManager(const IAssetProvider &assets);

    AssetDatabase &database() { return m_database; }
    const AssetDatabase &database() const { return m_database; }

    void loadBuiltins(IRenderResourceFactory &resources);
    void resolveScene(Scene &scene, IRenderResourceFactory &resources);

    /** Re-binds mesh/material handles from logical paths (call after scene load). */
    void resolveSceneAssets(Scene &scene, IRenderResourceFactory &resources)
    {
      resolveScene(scene, resources);
    }

    MeshHandle loadMesh(AssetGuid guid, IRenderResourceFactory &resources);
    MeshHandle loadMesh(std::string_view logicalPath, IRenderResourceFactory &resources);
    MaterialHandle loadMaterial(AssetGuid guid, IRenderResourceFactory &resources);
    MaterialHandle loadMaterial(std::string_view logicalPath, IRenderResourceFactory &resources);

    void unloadMesh(MeshHandle handle);
    void unloadMaterial(MaterialHandle handle);

    const Mesh *getMesh(MeshHandle handle) const;
    const Material *getMaterial(MaterialHandle handle) const;

    /** CPU mesh bounds used to fit box colliders to rendered geometry. */
    const MeshAsset *getCpuMeshAsset(std::string_view logicalPath) const;

    bool isMeshValid(MeshHandle handle) const;
    bool isMaterialValid(MaterialHandle handle) const;

    std::string_view meshPathForHandle(MeshHandle handle) const;
    std::string_view materialPathForHandle(MaterialHandle handle) const;

  private:
    void registerBuiltinCpuAssets();
    void loadBuiltinGpuAssets(IRenderResourceFactory &resources);

    const IAssetProvider &m_assets;
    AssetDatabase m_database;
    std::vector<Mesh> m_meshes;
    std::vector<Material> m_materials;
    std::unordered_map<AssetGuid, MeshHandle> m_meshGuidToHandle;
    std::unordered_map<MeshHandle, AssetGuid> m_meshHandleToGuid;
    std::unordered_map<AssetGuid, MaterialHandle> m_materialGuidToHandle;
    std::unordered_map<MaterialHandle, AssetGuid> m_materialHandleToGuid;
    bool m_builtinsLoaded = false;
  };
}
