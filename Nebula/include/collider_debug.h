#pragma once

#include "renderSystem.h"

namespace Nebula
{

  /** Draws wireframe boxes using the same world AABB as physics (`CollisionMath::worldAABBFromEntity`). */
  void renderColliderGizmos(const RenderSystemContext &ctx);

}
