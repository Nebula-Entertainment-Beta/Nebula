#pragma once

#include <string>
#include <vector>

#include "assetProvider.h"

namespace Editor
{

  enum class AssetEntryKind
  {
    Mesh,
    Material,
    Prefab
  };

  struct AssetEntry
  {
    AssetEntryKind kind = AssetEntryKind::Mesh;
    std::string logicalPath;
    std::string label;
  };

  class AssetCatalog
  {
  public:
    void refresh(const Nebula::FileAssetProvider &assets);

    const std::vector<AssetEntry> &entries() const { return m_entries; }

  private:
    std::vector<AssetEntry> m_entries;
  };

} // namespace Editor
