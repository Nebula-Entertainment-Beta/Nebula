#include "sceneSerializer.h"
#include "entityComponentJson_internal.h"
#include "assetManager.h"
#include <nlohmann/json.hpp>
#include <string>
#include <string_view>
#include <vector>

namespace Nebula
{
  namespace
  {
    constexpr EntityComponentJson::SerializeOptions kSceneSerializeOptions{
        .includeRuntimeIds = true,
        .includePrefabInstance = true,
        .includeCameraEntityRefs = true,
    };
  }

  bool SceneSerializer::save(const Scene &scene, const AssetManager &assetManager, const IAssetProvider &assets,
                             std::string_view logicalPath)
  {

    nlohmann::json root;
    root["version"] = kCurrentVersion;
    root["entities"] = nlohmann::json::array();

    for (const Entity &entity : scene.getAllEntities())
    {
      root["entities"].push_back(
          EntityComponentJson::serializeEntityToJson(scene, entity, assetManager, kSceneSerializeOptions));
    }

    const std::string text = root.dump(2);
    std::vector<uint8_t> bytes(text.begin(), text.end());
    return assets.writeFile(logicalPath, bytes);
  }

  bool SceneSerializer::load(Scene &scene, const IAssetProvider &assets, std::string_view logicalPath)
  {
    std::vector<uint8_t> bytes;
    if (!assets.readFile(logicalPath, bytes))
    {
      return false;
    }
    nlohmann::json root;
    try
    {
      const std::string json(bytes.begin(), bytes.end());
      root = nlohmann::json::parse(json);
    }
    catch (...)
    {
      return false;
    }
    const int version = root.value("version", 0);

    if (version < 0 || version > kCurrentVersion)
    {
      return false;
    }

    scene.clear();
    EntityID maxID = 0;
    if (!root.contains("entities") || !root["entities"].is_array())
    {
      return false;
    }

    for (const auto &entityJson : root["entities"])
    {
      if (!entityJson.contains("id") || !entityJson["id"].is_number_unsigned())
      {
        continue;
      }
      const EntityID id = entityJson["id"].get<EntityID>();
      const Entity entity = scene.registry().createEntityWithId(id);
      maxID = std::max(maxID, id);

      EntityComponentJson::deserializeEntityFromJson(scene, entity, entityJson);
    }

    scene.registry().setNextEntityId(maxID + 1);
    return true;
  }

  void SceneSerializer::saveToString(std::string &output, const Scene &scene, const AssetManager &assetManager)
  {
    nlohmann::json root;
    root["version"] = kCurrentVersion;
    root["entities"] = nlohmann::json::array();

    for (const Entity &entity : scene.getAllEntities())
    {
      root["entities"].push_back(
          EntityComponentJson::serializeEntityToJson(scene, entity, assetManager, kSceneSerializeOptions));
    }

    output = root.dump(2);
  }

  void SceneSerializer::loadFromString(Scene &scene, const std::string &input, const IAssetProvider &assets)
  {
    (void)assets;
    nlohmann::json root;
    try
    {
      root = nlohmann::json::parse(input);
    }
    catch (...)
    {
      return;
    }
    const int version = root.value("version", 0);

    if (version < 0 || version > kCurrentVersion)
    {
      return;
    }

    scene.clear();
    EntityID maxID = 0;
    if (!root.contains("entities") || !root["entities"].is_array())
    {
      return;
    }

    for (const auto &entityJson : root["entities"])
    {
      if (!entityJson.contains("id") || !entityJson["id"].is_number_unsigned())
      {
        continue;
      }
      const EntityID id = entityJson["id"].get<EntityID>();
      const Entity entity = scene.registry().createEntityWithId(id);
      maxID = std::max(maxID, id);

      EntityComponentJson::deserializeEntityFromJson(scene, entity, entityJson);
    }

    scene.registry().setNextEntityId(maxID + 1);
  }

} // namespace Nebula
