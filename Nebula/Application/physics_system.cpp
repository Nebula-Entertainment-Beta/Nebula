#include "physics/physics_system.h"
#include "physics/iphysics_world.h"
#include "scene.h"

namespace Nebula
{

  void runPhysicsFixedUpdate(Scene &scene, IPhysicsWorld &world, float fixedDt)
  {
    world.step(fixedDt);
    world.syncTransformsToScene(scene);
  }

} // namespace Nebula
