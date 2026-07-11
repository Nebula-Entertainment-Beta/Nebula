#pragma once

#include "ecs/entity.h"
#include "renderSystem.h"

namespace Nebula
{

  /** Draws RGB axis lines at @p selectedEntity world position (editor transform gizmo). */
  void renderTransformGizmo(const RenderSystemContext &ctx, Entity selectedEntity);

}
