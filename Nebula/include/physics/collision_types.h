#pragma once
#include "ecs/entity.h"
#include "math_types.h"

namespace Nebula
{

  struct RaycastHit
  {
    Entity entity{};
    Vec3 point{};
    Vec3 normal{};
    float distance = 0.0f;
  };

  struct OverlapHit
  {
    Entity entity{};
    Vec3 normal{};
    float penetration = 0.0f;
  };

  struct AABB
  {
    Vec3 min{};
    Vec3 max{};
  };

  struct Ray
  {
    Vec3 origin{};
    Vec3 direction{};
  };

  enum class OverlapFilter
  {
    All,
    SolidOnly,
    TriggersOnly,
  };

} // namespace Nebula
