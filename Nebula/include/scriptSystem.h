#pragma once
#include "ecs/entity.h"
#include "script.h"
#include "scene.h"
#include <map>

namespace Nebula
{
  class ScriptRegistry;

  class ScriptSystem
  {
  public:
    void rebuildFromScene(Scene &scene, ScriptRegistry &registry, ScriptContext &ctx);
    /** Binds scripts for scene entities not yet in m_instances; does not reset existing scripts. */
    void bindNewFromScene(Scene &scene, ScriptRegistry &registry, ScriptContext &ctx, bool activate = true);
    void initializeAll(ScriptContext &ctx);
    void updateAll(ScriptContext &ctx, float dt);
    void physicsUpdateAll(ScriptContext &ctx, float fixedDt);
    void renderAll(ScriptContext &ctx, float dt);
    void shutdownAll(ScriptContext &ctx);
    bool hasInstance(Entity entity) const { return m_instances.find(entity) != m_instances.end(); }

  private:
    // Ordered by entity id for deterministic lifecycle/update order.
    std::map<Entity, ScriptPtr> m_instances;
  };
} // namespace Nebula
