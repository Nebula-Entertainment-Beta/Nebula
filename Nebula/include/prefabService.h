#pragma once

#include <optional>
#include <string>
#include <string_view>

#include "ecs/entity.h"
#include "scene.h"
#include "assetManager.h"
#include "assetProvider.h"
#include "prefabInstance.h"

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

    static bool revertInstance(Scene &scene, Entity entity, AssetManager &assets,
                               const IAssetProvider &fileProvider,
                               IRenderResourceFactory &renderResources);

    static bool saveVariantFromInstance(const Scene &scene, Entity entity, const AssetManager &assets,
                                        const IAssetProvider &fileProvider, std::string_view variantPath);

    static std::optional<std::string> resolvePrefabEntityJsonString(const IAssetProvider &fileProvider,
                                                                    std::string_view prefabPath);

    static std::string computeInstanceOverridesJson(const Scene &scene, Entity entity,
                                                    const AssetManager &assets,
                                                    const IAssetProvider &fileProvider);

    static void setInstanceOverrideField(PrefabInstanceComponent &instance, const char *componentKey,
                                         const char *fieldKey, float value);
    static void setInstanceOverrideField(PrefabInstanceComponent &instance, const char *componentKey,
                                         const char *fieldKey, const float *vec3Value);
    static void setInstanceOverrideField(PrefabInstanceComponent &instance, const char *componentKey,
                                         const char *fieldKey, int value);
    static void setInstanceOverrideField(PrefabInstanceComponent &instance, const char *componentKey,
                                         const char *fieldKey, bool value);
    static void setInstanceOverrideFieldJson(PrefabInstanceComponent &instance, const char *componentKey,
                                             const char *fieldKey, std::string_view jsonValue);

    static void clearInstanceOverrideField(PrefabInstanceComponent &instance, const char *componentKey,
                                           const char *fieldKey);

    static bool overridesContainPath(std::string_view overridesJson, const char *componentKey,
                                     const char *fieldKey);

    static bool resetInstanceOverrideField(Scene &scene, Entity entity, const AssetManager &assets,
                                          const IAssetProvider &fileProvider, const char *componentKey,
                                          const char *fieldKey);
  };
}
