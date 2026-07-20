#include "component.h"
#include "scene.h"
#include "scene_query.h"

namespace Nebula
{

  EnvironmentComponent findEnvironmentOrDefault(const Scene &scene)
  {
    for (const Entity e : scene.getAllEntities())
    {
      if (scene.hasComponent<EnvironmentComponent>(e))
      {
        return scene.getComponent<EnvironmentComponent>(e);
      }
    }
    return {};
  }

  Vec4 environmentClearColor(const EnvironmentComponent &env)
  {
    switch (env.timeOfDayPreset)
    {
    case 1: // dusk
      return Vec4{0.55f, 0.35f, 0.45f, 1.f};
    case 2: // night
      return Vec4{0.05f, 0.07f, 0.15f, 1.f};
    default:
      return Vec4{(env.skyTop.x + env.skyBottom.x) * 0.5f,
                  (env.skyTop.y + env.skyBottom.y) * 0.5f,
                  (env.skyTop.z + env.skyBottom.z) * 0.5f, 1.f};
    }
  }

}
