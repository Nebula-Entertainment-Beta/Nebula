#pragma once

#include "scene.h"

namespace Nimbus
{

  /** Populates a scene with ground, player cube, and main camera (Week 2 default). */
  void buildDefaultScene(Nebula::Scene &scene);

  /** Combat-focused starter scene (same content as default for now). */
  void buildCombatArenaScene(Nebula::Scene &scene);

  /** Platforming test room with bounce pads, wind volumes, spawn, and goal. */
  void buildTraversalTestScene(Nebula::Scene &scene);

  /** Combined traversal + arena combat + objective vertical slice. */
  void buildVerticalSliceScene(Nebula::Scene &scene);

}
