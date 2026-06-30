#pragma once

#include <string_view>

#include "ecs/entity.h"
#include "scene.h"
#include "assetManager.h"
#include "assetProvider.h"

namespace Nebula
{
  class IRenderResourceFactory;

  class PrefabService
  {
  public:
    static Entity instantiate(Scene &scene, AssetManager &assets,
                              const IAssetProvider &fileProvider,
                              IRenderResourceFactory &renderResources,
                              std::string_view prefabPath);
  };
}
