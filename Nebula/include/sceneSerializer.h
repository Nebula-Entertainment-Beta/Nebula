#pragma once
#include <string_view>
#include "scene.h"
#include "assetProvider.h"

namespace Nebula
{

  class SceneSerializer
  {
  public:
    static constexpr int kCurrentVersion = 1;

    static bool save(const Scene &scene, const IAssetProvider &assets, std::string_view logicalPath);
    static bool load(Scene &scene, const IAssetProvider &assets, std::string_view logicalPath);
  };

}