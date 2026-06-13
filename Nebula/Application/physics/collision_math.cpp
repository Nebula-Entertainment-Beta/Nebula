#include "physics/collision_math.h"
#include "physics/physics_component.h"

#include <algorithm>
#include <cmath>
#include <cstdint>

namespace Nebula
{

  namespace
  {
    Vec3 transformPoint(const Mat4 &m, const Vec3 &p)
    {
      return {
          m.cols[0] * p.x + m.cols[4] * p.y + m.cols[8] * p.z + m.cols[12],
          m.cols[1] * p.x + m.cols[5] * p.y + m.cols[9] * p.z + m.cols[13],
          m.cols[2] * p.x + m.cols[6] * p.y + m.cols[10] * p.z + m.cols[14],
      };
    }

    AABB boundsFromLocalHalfExtents(const Transform3D &transform, const Vec3 &localHalf)
    {
      const Mat4 model = transform.getModelMatrix();
      const Vec3 corners[8] = {
          {-localHalf.x, -localHalf.y, -localHalf.z},
          {localHalf.x, -localHalf.y, -localHalf.z},
          {-localHalf.x, localHalf.y, -localHalf.z},
          {localHalf.x, localHalf.y, -localHalf.z},
          {-localHalf.x, -localHalf.y, localHalf.z},
          {localHalf.x, -localHalf.y, localHalf.z},
          {-localHalf.x, localHalf.y, localHalf.z},
          {localHalf.x, localHalf.y, localHalf.z},
      };
      Vec3 worldMin = transformPoint(model, corners[0]);
      Vec3 worldMax = worldMin;
      for (int i = 1; i < 8; ++i)
      {
        const Vec3 p = transformPoint(model, corners[i]);
        worldMin.x = std::min(worldMin.x, p.x);
        worldMin.y = std::min(worldMin.y, p.y);
        worldMin.z = std::min(worldMin.z, p.z);
        worldMax.x = std::max(worldMax.x, p.x);
        worldMax.y = std::max(worldMax.y, p.y);
        worldMax.z = std::max(worldMax.z, p.z);
      }
      return {worldMin, worldMax};
    }

    float inverseDirectionComponent(float component)
    {
      constexpr float kEpsilon = 1e-8f;
      if (std::abs(component) < kEpsilon)
      {
        return component >= 0.0f ? 1e30f : -1e30f;
      }
      return 1.0f / component;
    }

    float axisOverlap(float minA, float maxA, float minB, float maxB)
    {
      return std::min(maxA, maxB) - std::max(minA, minB);
    }
  }

  bool CollisionMath::rayVsAABB(const Ray &ray, const AABB &box, float &t) const
  {
    const Vec3 invDir{
        inverseDirectionComponent(ray.direction.x),
        inverseDirectionComponent(ray.direction.y),
        inverseDirectionComponent(ray.direction.z),
    };

    float tMin = 0.0f;
    float tMax = 1e30f;

    const float tx1 = (box.min.x - ray.origin.x) * invDir.x;
    const float tx2 = (box.max.x - ray.origin.x) * invDir.x;
    tMin = std::max(tMin, std::min(tx1, tx2));
    tMax = std::min(tMax, std::max(tx1, tx2));

    const float ty1 = (box.min.y - ray.origin.y) * invDir.y;
    const float ty2 = (box.max.y - ray.origin.y) * invDir.y;
    tMin = std::max(tMin, std::min(ty1, ty2));
    tMax = std::min(tMax, std::max(ty1, ty2));

    const float tz1 = (box.min.z - ray.origin.z) * invDir.z;
    const float tz2 = (box.max.z - ray.origin.z) * invDir.z;
    tMin = std::max(tMin, std::min(tz1, tz2));
    tMax = std::min(tMax, std::max(tz1, tz2));

    if (tMax < 0.0f || tMin > tMax)
    {
      return false;
    }

    t = tMin >= 0.0f ? tMin : tMax;
    return true;
  }

  bool CollisionMath::rayVsSphere(const Ray &ray, const Vec3 &center, float radius, float &t) const
  {
    const Vec3 oc{ray.origin.x - center.x, ray.origin.y - center.y, ray.origin.z - center.z};
    const float b = ray.direction.x * oc.x + ray.direction.y * oc.y + ray.direction.z * oc.z;
    const float c = oc.x * oc.x + oc.y * oc.y + oc.z * oc.z - radius * radius;
    const float discriminant = b * b - c;
    if (discriminant < 0.0f)
    {
      return false;
    }

    const float sqrtDisc = std::sqrt(discriminant);
    const float t0 = -b - sqrtDisc;
    const float t1 = -b + sqrtDisc;
    if (t0 >= 0.0f)
    {
      t = t0;
      return true;
    }
    if (t1 >= 0.0f)
    {
      t = t1;
      return true;
    }
    return false;
  }

  Vec3 CollisionMath::aabbNormalAtPoint(const AABB &box, const Vec3 &point) const
  {
    const float toMinX = std::abs(point.x - box.min.x);
    const float toMaxX = std::abs(point.x - box.max.x);
    const float toMinY = std::abs(point.y - box.min.y);
    const float toMaxY = std::abs(point.y - box.max.y);
    const float toMinZ = std::abs(point.z - box.min.z);
    const float toMaxZ = std::abs(point.z - box.max.z);

    const float minDist = std::min({toMinX, toMaxX, toMinY, toMaxY, toMinZ, toMaxZ});
    if (minDist == toMinX)
    {
      return {-1.0f, 0.0f, 0.0f};
    }
    if (minDist == toMaxX)
    {
      return {1.0f, 0.0f, 0.0f};
    }
    if (minDist == toMinY)
    {
      return {0.0f, -1.0f, 0.0f};
    }
    if (minDist == toMaxY)
    {
      return {0.0f, 1.0f, 0.0f};
    }
    if (minDist == toMinZ)
    {
      return {0.0f, 0.0f, -1.0f};
    }
    return {0.0f, 0.0f, 1.0f};
  }

  bool CollisionMath::sphereVsAABB(const Vec3 &center, float radius, const AABB &box, OverlapHit &out) const
  {
    const Vec3 closest{
        std::clamp(center.x, box.min.x, box.max.x),
        std::clamp(center.y, box.min.y, box.max.y),
        std::clamp(center.z, box.min.z, box.max.z),
    };

    const Vec3 delta{center.x - closest.x, center.y - closest.y, center.z - closest.z};
    const float distSq = delta.x * delta.x + delta.y * delta.y + delta.z * delta.z;
    const float radiusSq = radius * radius;

    if (distSq > radiusSq)
    {
      return false;
    }

    const float dist = std::sqrt(distSq);
    if (dist > 1e-8f)
    {
      out.normal = {delta.x / dist, delta.y / dist, delta.z / dist};
      out.penetration = radius - dist;
      return true;
    }

    const float penX = std::min(box.max.x - center.x, center.x - box.min.x);
    const float penY = std::min(box.max.y - center.y, center.y - box.min.y);
    const float penZ = std::min(box.max.z - center.z, center.z - box.min.z);

    if (penX <= penY && penX <= penZ)
    {
      out.penetration = penX;
      out.normal = {center.x < (box.min.x + box.max.x) * 0.5f ? -1.0f : 1.0f, 0.0f, 0.0f};
    }
    else if (penY <= penZ)
    {
      out.penetration = penY;
      out.normal = {0.0f, center.y < (box.min.y + box.max.y) * 0.5f ? -1.0f : 1.0f, 0.0f};
    }
    else
    {
      out.penetration = penZ;
      out.normal = {0.0f, 0.0f, center.z < (box.min.z + box.max.z) * 0.5f ? -1.0f : 1.0f};
    }

    return true;
  }

  bool CollisionMath::aabbVsAABB(const AABB &a, const AABB &b, OverlapHit &out) const
  {
    if (a.max.x < b.min.x || a.min.x > b.max.x ||
        a.max.y < b.min.y || a.min.y > b.max.y ||
        a.max.z < b.min.z || a.min.z > b.max.z)
    {
      return false;
    }

    const float overlapX = axisOverlap(a.min.x, a.max.x, b.min.x, b.max.x);
    const float overlapY = axisOverlap(a.min.y, a.max.y, b.min.y, b.max.y);
    const float overlapZ = axisOverlap(a.min.z, a.max.z, b.min.z, b.max.z);

    const float centerAx = (a.min.x + a.max.x) * 0.5f;
    const float centerAy = (a.min.y + a.max.y) * 0.5f;
    const float centerAz = (a.min.z + a.max.z) * 0.5f;
    const float centerBx = (b.min.x + b.max.x) * 0.5f;
    const float centerBy = (b.min.y + b.max.y) * 0.5f;
    const float centerBz = (b.min.z + b.max.z) * 0.5f;

    if (overlapX <= overlapY && overlapX <= overlapZ)
    {
      out.penetration = overlapX;
      out.normal = {centerAx < centerBx ? -1.0f : 1.0f, 0.0f, 0.0f};
    }
    else if (overlapY <= overlapZ)
    {
      out.penetration = overlapY;
      out.normal = {0.0f, centerAy < centerBy ? -1.0f : 1.0f, 0.0f};
    }
    else
    {
      out.penetration = overlapZ;
      out.normal = {0.0f, 0.0f, centerAz < centerBz ? -1.0f : 1.0f};
    }

    return true;
  }

  AABB CollisionMath::worldAABBFromEntity(Scene &scene, Entity entity) const
  {
    if (!scene.isValidEntity(entity) ||
        !scene.hasComponent<TransformComponent>(entity) ||
        !scene.hasComponent<ColliderComponent>(entity))
    {
      return {};
    }
    const Transform3D &transform = scene.getComponent<TransformComponent>(entity).transform;
    const ColliderComponent &collider = scene.getComponent<ColliderComponent>(entity);
    if (collider.shape == ColliderComponent::Shape::Sphere)
    {
      const float radius = collider.halfExtents.x * transform.getScale();
      const Vec3 center = transform.getPosition();
      return {
          {center.x - radius, center.y - radius, center.z - radius},
          {center.x + radius, center.y + radius, center.z + radius},
      };
    }
    return boundsFromLocalHalfExtents(transform, collider.halfExtents);
  }

}