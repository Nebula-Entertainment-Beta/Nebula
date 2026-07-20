/**
 * @file scene_query.h
 * @brief Engine lookup helpers: by tag or by component type.
 */
#pragma once

#include <string_view>
#include <vector>

#include "ecs/entity.h"
#include "scene.h"

namespace Nebula
{

  Entity findByTag(Scene &scene, std::string_view tag);

  /** First camera with isPrimary, else the first camera in the scene. */
  Entity findPrimaryCameraEntity(Scene &scene);

  template <typename ComponentType>
  std::vector<Entity> findAllWith(const Scene &scene)
  {
    std::vector<Entity> result;
    for (const Entity entity : scene.getAllEntities())
    {
      if (scene.hasComponent<ComponentType>(entity))
      {
        result.push_back(entity);
      }
    }
    return result;
  }

  std::vector<Entity> findAllByTag(Scene &scene, std::string_view tag);

} // namespace Nebula
