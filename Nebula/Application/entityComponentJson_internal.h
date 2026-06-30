#pragma once

#include "entityComponentJson.h"

#include <nlohmann/json.hpp>

namespace Nebula::EntityComponentJson
{

  nlohmann::json serializeEntityToJson(const Scene &scene, Entity entity, const AssetManager &assets,
                                       SerializeOptions opts = {});

  void deserializeEntityFromJson(Scene &scene, Entity entity, const nlohmann::json &entityJson);

} // namespace Nebula::EntityComponentJson
