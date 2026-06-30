#include "prefabSerializer.h"

#include "entityComponentJson_internal.h"

#include <nlohmann/json.hpp>
#include <string>
#include <vector>

namespace Nebula
{

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

} // namespace Nebula
