#pragma once

#include "ecs/entity.h"
#include "renderSystem.h"

namespace Nebula
{

  class Scene;

  /** World-space axis length for W/E/R handles — scales with the entity's bounds. */
  float transformGizmoAxisLength(Scene &scene, Entity entity);

  /** Draws RGB axis handles at @p selectedEntity (0=translate arrows, 1=rotate rings, 2=scale cubes). */
  void renderTransformGizmo(const RenderSystemContext &ctx, Entity selectedEntity, int gizmoMode = 0);

}
