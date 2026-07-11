#pragma once
#include <string_view>
#include "scene.h"
#include "assetManager.h"
#include "assetProvider.h"
#include "ecs/entity.h"

namespace Nebula
{
  class PrefabSerializer
  {
  public:
    static constexpr int kCurrentVersion = 1;

    static bool save(const Scene &scene, Entity entity, const AssetManager &assets,
                     const IAssetProvider &fileProvider, std::string_view logicalPath);

    static bool saveVariant(const IAssetProvider &fileProvider, std::string_view logicalPath,
                            std::string_view basePrefabPath, std::string_view overridesJson);

    static bool loadRootJsonString(const IAssetProvider &fileProvider, std::string_view logicalPath,
                                   std::string &outJson);
  };
}
