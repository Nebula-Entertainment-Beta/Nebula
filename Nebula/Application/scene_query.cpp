#include "scene_query.h"
#include "component.h"
#include "scene.h"
#include "tag_component.h"

namespace Nebula
{

  Entity findByTag(Scene &scene, std::string_view tag)
  {
    for (const Entity entity : scene.getAllEntities())
    {
      if (!scene.hasComponent<TagComponent>(entity))
      {
        continue;
      }
      if (scene.getComponent<TagComponent>(entity).tag == tag)
      {
        return entity;
      }
    }
    return {};
  }

  Entity findPrimaryCameraEntity(Scene &scene)
  {
    Entity fallback{};
    for (const Entity entity : scene.getAllEntities())
    {
      if (!scene.hasComponent<CameraComponent>(entity))
      {
        continue;
      }
      if (scene.getComponent<CameraComponent>(entity).isPrimary)
      {
        return entity;
      }
      if (fallback.id == 0)
      {
        fallback = entity;
      }
    }
    return fallback;
  }

  std::vector<Entity> findAllByTag(Scene &scene, std::string_view tag)
  {
    std::vector<Entity> entities;
    for (const Entity entity : scene.getAllEntities())
    {
      if (!scene.hasComponent<TagComponent>(entity))
      {
        continue;
      }
      if (scene.getComponent<TagComponent>(entity).tag == tag)
      {
        entities.push_back(entity);
      }
    }
    return entities;
  }

} // namespace Nebula
