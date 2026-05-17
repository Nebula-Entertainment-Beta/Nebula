#pragma once

#include "component.h"
#include "scene.h"
#include "world.h"

namespace Nimbus
{
  void runPlayerController(Nebula::World &world,
                           Nebula::Entity cubeEntity,
                           Nebula::Entity cameraEntity,
                           bool &showInputDebug,
                           float &debugPrintTimer,
                           float dt);
}
