#pragma once

#include "ecs/entity.h"
#include "renderSystem.h"

namespace Nebula
{

  /** Draws RGB axis handles at @p selectedEntity (0=translate arrows, 1=rotate rings, 2=scale cubes). */
  void renderTransformGizmo(const RenderSystemContext &ctx, Entity selectedEntity, int gizmoMode = 0);

}
