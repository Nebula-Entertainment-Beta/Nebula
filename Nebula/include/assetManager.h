#pragma once
#include <vector>
#include "mesh.h"
#include "material.h"
#include "assetProvider.h"

namespace Nebula
{
  class IAssetProvider;

  class AssetManager
  {
  public:
    void loadBuiltins(const IAssetProvider &fileAssets);

    const Mesh *getMesh(MeshHandle handle) const;
    const Material *getMaterial(MaterialHandle handle) const;

    bool isMeshValid(MeshHandle handle) const;
    bool isMaterialValid(MaterialHandle handle) const;

  private:
    void loadBuiltinMeshes();
    void loadBuiltinMaterials(const IAssetProvider &fileAssets);

    std::vector<Mesh> m_meshes;
    std::vector<Material> m_materials;
    bool m_builtinsLoaded = false;
  };
}