/**
 * @file entityComponentJson.h
 * @brief Serialize/deserialize a single entity's components to JSON text (shared by scene and prefab I/O).
 */
#pragma once

#include <string>
#include <string_view>

#include "ecs/entity.h"
#include "scene.h"

namespace Nebula
{
  class AssetManager;

  namespace EntityComponentJson
  {

    struct SerializeOptions
    {
      bool includeRuntimeIds = false;
      bool includePrefabInstance = false;
      bool includeCameraEntityRefs = false;
    };

    /** @return JSON object text for one entity (no surrounding array or file wrapper). */
    std::string serializeEntity(const Scene &scene, Entity entity, const AssetManager &assets,
                                SerializeOptions opts = {});

    /** @param entityJsonText JSON object text produced by @ref serializeEntity. */
    void deserializeEntity(Scene &scene, Entity entity, std::string_view entityJsonText);

  } // namespace EntityComponentJson

} // namespace Nebula
