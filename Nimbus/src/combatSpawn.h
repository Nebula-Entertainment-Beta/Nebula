#pragma once

#include "math_types.h"
#include "script.h"

namespace Nimbus
{

  /** Creates an enemy entity from prefab when spawn context is available, otherwise inline setup. */
  Nebula::Entity spawnEnemy(Nebula::ScriptContext &ctx, const Nebula::Vec3 &position);

}
