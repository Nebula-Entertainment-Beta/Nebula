/**
 * @file iphysics_world.h
 * @brief Physics simulation boundary; backend implementations (null, Jolt, …) stay in .cpp.
 */
#pragma once

#include "ecs/entity.h"
#include "math_types.h"
#include <memory>

namespace Nebula
{

  class Scene;

  struct RaycastHit
  {
    Entity entity{};
    Vec3 point{};
    Vec3 normal{};
    float distance = 0.0f;
  };

  class IPhysicsWorld
  {
  public:
    virtual ~IPhysicsWorld() = default;

    virtual void step(float fixedDt) = 0;
    virtual void syncTransformsToScene(Scene &scene) = 0;

    virtual void setBodyPosition(Scene &scene, Entity entity, const Vec3 &position) = 0;
    virtual Vec3 getBodyPosition(const Scene &scene, Entity entity) const = 0;

    virtual bool raycast(const Scene &scene, const Vec3 &origin, const Vec3 &direction,
                         float maxDistance, RaycastHit &out) const = 0;
  };

  std::unique_ptr<IPhysicsWorld> createNullPhysicsWorld();

} // namespace Nebula
