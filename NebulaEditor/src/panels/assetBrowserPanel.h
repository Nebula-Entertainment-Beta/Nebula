#pragma once

#include <functional>
#include <string>
#include <vector>

#include "assetCatalog.h"
#include "editorState.h"

namespace Nebula
{
  class AssetManager;
  class Scene;
}

namespace Editor
{

  class AssetBrowserPanel
  {
  public:
    void draw(AssetCatalog &catalog, EditorState &state, Nebula::Scene &scene,
              Nebula::AssetManager &assets,
              const std::function<void(std::string_view meshPath, std::string_view materialPath)> &spawnMesh,
              const std::function<void(std::string_view prefabPath)> &spawnPrefab,
              const std::function<void()> &refreshCatalog);
  };

} // namespace Editor
