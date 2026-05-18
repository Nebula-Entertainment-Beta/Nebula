/**
 * @file physics_component.h
 * @brief ECS data for rigid bodies and colliders; simulation lives in `IPhysicsWorld`.
 */
#pragma once

#include "math_types.h"

namespace Nebula
{

  struct RigidBodyComponent
  {
    float mass = 1.0f;
    bool kinematic = false;
  };

  struct ColliderComponent
  {
    enum class Shape
    {
      Box,
      Sphere
    };

    Shape shape = Shape::Box;
    Vec3 halfExtents{0.5f, 0.5f, 0.5f};
  };

} // namespace Nebula
