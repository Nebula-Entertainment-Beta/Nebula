#include "traversalDirector.h"
#include "nimbusRuntime.h"

namespace Nimbus
{

  void TraversalDirector::onCreate(Nebula::ScriptContext &ctx, Nebula::Entity self)
  {
    if (!ctx.scene.isValidEntity(self))
      return;
    const Nebula::ScriptComponent &sc = ctx.scene.getScriptComponent(self);
    m_lastParamsJson = sc.paramsJson;
    syncTraversalFromParams(sc, m_params, traversal(ctx));
  }

  void TraversalDirector::onEnable(Nebula::ScriptContext &ctx, Nebula::Entity self)
  {
    onCreate(ctx, self);
  }

  void TraversalDirector::onUpdate(Nebula::ScriptContext &ctx, Nebula::Entity self, float)
  {
    if (!ctx.scene.isValidEntity(self))
      return;
    const Nebula::ScriptComponent &sc = ctx.scene.getScriptComponent(self);
    if (sc.paramsJson == m_lastParamsJson)
      return;
    m_lastParamsJson = sc.paramsJson;
    syncTraversalFromParams(sc, m_params, traversal(ctx));
  }
}
