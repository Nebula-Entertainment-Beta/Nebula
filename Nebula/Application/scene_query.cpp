#include "scene_query.h"
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

} // namespace Nebula
