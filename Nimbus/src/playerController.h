#pragma once

#include "component.h"
#include "scene.h"
#include "world.h"
#include "eventBus.h"

namespace Nimbus
{
  void runPlayerController(Nebula::World &world,
                           Nebula::EventBus &events,
                           Nebula::Entity cubeEntity,
                           Nebula::Entity cameraEntity,
                           bool &showInputDebug,
                           float &debugPrintTimer,
                           float dt);
}
