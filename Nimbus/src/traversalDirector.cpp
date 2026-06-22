#include "traversalDirector.h"

namespace Nimbus
{

  void TraversalDirector::onCreate(Nebula::ScriptContext &ctx, Nebula::Entity self)
  {
    if (!ctx.scene.isValidEntity(self))
      return;
    syncTraversalFromParams(ctx.scene.getScriptComponent(self), m_params, m_settings);
  }

  void TraversalDirector::onEnable(Nebula::ScriptContext &ctx, Nebula::Entity self)
  {
    onCreate(ctx, self);
  }

  void TraversalDirector::onUpdate(Nebula::ScriptContext &ctx, Nebula::Entity self, float)
  {
    if (!ctx.scene.isValidEntity(self))
      return;
    syncTraversalFromParams(ctx.scene.getScriptComponent(self), m_params, m_settings);
  }
}