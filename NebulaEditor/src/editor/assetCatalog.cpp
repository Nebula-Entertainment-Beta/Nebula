#include "assetCatalog.h"

#include "assetProvider.h"

namespace Editor
{

  void AssetCatalog::refresh(const Nebula::FileAssetProvider &assets)
  {
    m_entries.clear();

    const auto meshes = assets.listFiles("meshes", {"mesh", "obj", "fbx"});
    for (const std::string &path : meshes)
    {
      m_entries.push_back({AssetEntryKind::Mesh, path, path});
    }

    const auto materials = assets.listFiles("materials", {"mat"});
    for (const std::string &path : materials)
    {
      m_entries.push_back({AssetEntryKind::Material, path, path});
    }

    const auto prefabs = assets.listFiles("prefabs", {"prefab"});
    for (const std::string &path : prefabs)
    {
      m_entries.push_back({AssetEntryKind::Prefab, path, path});
    }
  }

} // namespace Editor
