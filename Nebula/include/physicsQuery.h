/**
 * @file physicsQuery.h
 * @brief Narrow script-facing collision API; hides `IPhysicsWorld` simulation types.
 */
#pragma once

#include "ecs/entity.h"
#include "math_types.h"
#include "physics/collision_types.h"

#include <vector>

namespace Nebula
{

  class Scene;

  class IPhysicsQuery
  {
  public:
    virtual ~IPhysicsQuery() = default;

    virtual bool raycast(const Scene &scene, const Vec3 &origin, const Vec3 &direction, float maxDistance,
                         RaycastHit &out) const = 0;

    virtual bool overlapBox(const Scene &scene, const Vec3 &center, const Vec3 &halfExtents,
                            std::vector<OverlapHit> &out,
                            OverlapFilter filter = OverlapFilter::All) const = 0;

    virtual void moveKinematic(Scene &scene, Entity entity, const Vec3 &delta, bool &outGrounded) = 0;

    virtual bool isGrounded(const Scene &scene, Entity entity) const = 0;
  };

} // namespace Nebula
