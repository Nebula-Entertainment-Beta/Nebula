#pragma once
#include <string_view>
#include "scene.h"
#include "assetManager.h"
#include "assetProvider.h"

namespace Nebula
{
  class AssetManager;

  class SceneSerializer
  {
  public:
    static constexpr int kCurrentVersion = 2;

    static bool save(const Scene &scene, const AssetManager &assets, const IAssetProvider &fileProvider,
                     std::string_view logicalPath);
    static bool load(Scene &scene, const IAssetProvider &assets, std::string_view logicalPath);

    void saveToString(std::string &output, const Scene &scene, const AssetManager &assets);
    void loadFromString(Scene &scene, const std::string &input, const IAssetProvider &assets);
  };

}
