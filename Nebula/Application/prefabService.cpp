#include "prefabService.h"

#include "entityComponentJson_internal.h"
#include "prefabInstance.h"
#include "prefabSerializer.h"
#include "renderResources.h"

#include <nlohmann/json.hpp>
#include <string>
#include <vector>

namespace Nebula
{

  Entity PrefabService::instantiate(Scene &scene, AssetManager &assets,
                                    const IAssetProvider &fileProvider,
                                    IRenderResourceFactory &renderResources,
                                    std::string_view prefabPath)
  {
    std::vector<uint8_t> bytes;
    if (!fileProvider.readFile(prefabPath, bytes))
    {
      return {};
    }

    nlohmann::json root;
    try
    {
      root = nlohmann::json::parse(std::string(bytes.begin(), bytes.end()));
    }
    catch (...)
    {
      return {};
    }

    if (root.value("version", 0) != PrefabSerializer::kCurrentVersion)
    {
      return {};
    }
    if (!root.contains("entity") || !root["entity"].is_object())
    {
      return {};
    }

    const Entity entity = scene.createEntity();
    EntityComponentJson::deserializeEntityFromJson(scene, entity, root["entity"]);

    auto &inst = scene.addComponent<PrefabInstanceComponent>(entity);
    inst.prefabPath = std::string(prefabPath);
    inst.overridesJson = "{}";

    assets.resolveScene(scene, renderResources);

    return entity;
  }

} // namespace Nebula
