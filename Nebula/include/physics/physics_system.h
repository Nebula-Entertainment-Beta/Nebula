/**
 * @file physics_system.h
 * @brief Fixed-timestep physics tick: simulate, then write poses into `TransformComponent`.
 */
#pragma once

namespace Nebula
{

  class Scene;
  class IPhysicsWorld;

  void runPhysicsFixedUpdate(Scene &scene, IPhysicsWorld &world, float fixedDt);

} // namespace Nebula
