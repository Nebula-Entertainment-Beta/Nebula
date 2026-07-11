#include "prefabSerializer.h"

#include "entityComponentJson_internal.h"

#include <nlohmann/json.hpp>
#include <string>
#include <vector>

namespace Nebula
{

  bool PrefabSerializer::loadRootJsonString(const IAssetProvider &fileProvider, std::string_view logicalPath,
                                            std::string &outJson)
  {
    std::vector<uint8_t> bytes;
    if (!fileProvider.readFile(logicalPath, bytes))
    {
      return false;
    }
    outJson.assign(bytes.begin(), bytes.end());
    try
    {
      const nlohmann::json root = nlohmann::json::parse(outJson);
      return root.value("version", 0) == kCurrentVersion;
    }
    catch (...)
    {
      return false;
    }
  }

  bool PrefabSerializer::save(const Scene &scene, Entity entity, const AssetManager &assets,
                              const IAssetProvider &fileProvider, std::string_view logicalPath)
  {
    if (!scene.isValidEntity(entity))
    {
      return false;
    }

    nlohmann::json root;
    root["version"] = kCurrentVersion;
    root["entity"] = EntityComponentJson::serializeEntityToJson(scene, entity, assets, {
                                                                                          .includeRuntimeIds = false,
                                                                                          .includePrefabInstance = false,
                                                                                          .includeCameraEntityRefs = false,
                                                                                      });

    const std::string text = root.dump(2);
    const std::vector<uint8_t> bytes(text.begin(), text.end());
    return fileProvider.writeFile(logicalPath, bytes);
  }

  bool PrefabSerializer::saveVariant(const IAssetProvider &fileProvider, std::string_view logicalPath,
                                     std::string_view basePrefabPath, std::string_view overridesJson)
  {
    nlohmann::json overrides = nlohmann::json::object();
    if (!overridesJson.empty())
    {
      try
      {
        overrides = nlohmann::json::parse(overridesJson);
      }
      catch (...)
      {
        return false;
      }
    }

    nlohmann::json root;
    root["version"] = kCurrentVersion;
    root["basePrefab"] = std::string(basePrefabPath);
    root["overrides"] = overrides;
    const std::string text = root.dump(2);
    const std::vector<uint8_t> bytes(text.begin(), text.end());
    return fileProvider.writeFile(logicalPath, bytes);
  }

} // namespace Nebula
