/**
 * @file iphysics_world.h
 * @brief Physics simulation boundary; backend implementations (simple, Jolt, …) stay in .cpp.
 */
#pragma once

#include "ecs/entity.h"
#include "math_types.h"
#include "physics/collision_types.h"
#include <memory>
#include <vector>

namespace Nebula
{

  class Scene;

  class IPhysicsWorld
  {
  public:
    virtual ~IPhysicsWorld() = default;

    virtual void step(Scene &scene, float fixedDt) = 0;
    virtual void syncTransformsToScene(Scene &scene) = 0;

    virtual void setBodyPosition(Scene &scene, Entity entity, const Vec3 &position) = 0;
    virtual Vec3 getBodyPosition(const Scene &scene, Entity entity) const = 0;

    virtual bool raycast(const Scene &scene, const Vec3 &origin, const Vec3 &direction,
                         float maxDistance, RaycastHit &out) const = 0;

    virtual bool overlapSphere(const Scene &scene, const Vec3 &center, float radius,
                               std::vector<OverlapHit> &out,
                               OverlapFilter filter = OverlapFilter::All) const = 0;

    virtual bool overlapBox(const Scene &scene, const Vec3 &center, const Vec3 &halfExtents,
                            std::vector<OverlapHit> &out,
                            OverlapFilter filter = OverlapFilter::All) const = 0;

    virtual void moveKinematic(Scene &scene, Entity entity, const Vec3 &delta, bool &outGrounded) = 0;
  };

  std::unique_ptr<IPhysicsWorld> createSimplePhysicsWorld();
  std::unique_ptr<IPhysicsWorld> createNullPhysicsWorld();

} // namespace Nebula
