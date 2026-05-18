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

} // namespace Nebula
