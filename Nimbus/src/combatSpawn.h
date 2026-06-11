#pragma once

#include "math_types.h"
#include "sceneAccess.h"

namespace Nimbus
{

  /** Creates a fully configured enemy entity (tag, transform, mesh, Enemy script). */
  Nebula::Entity spawnEnemy(Nebula::ISceneAccess &scene, const Nebula::Vec3 &position);

}
