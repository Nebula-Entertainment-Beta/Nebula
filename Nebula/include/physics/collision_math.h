#pragma once
#include "ecs/entity.h"
#include "scene.h"
#include "physics/collision_types.h"

namespace Nebula
{

  class CollisionMath
  {
  public:
    bool rayVsAABB(const Ray &ray, const AABB &box, float &t) const;
    bool rayVsSphere(const Ray &ray, const Vec3 &center, float radius, float &t) const;
    bool sphereVsAABB(const Vec3 &center, float radius, const AABB &box, OverlapHit &out) const;
    bool aabbVsAABB(const AABB &a, const AABB &b, OverlapHit &out) const;

    AABB worldAABBFromEntity(Scene &scene, Entity entity) const;
    Vec3 aabbNormalAtPoint(const AABB &box, const Vec3 &point) const;
  };

} // namespace Nebula
