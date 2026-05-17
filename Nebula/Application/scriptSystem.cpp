#include "scriptSystem.h"
#include "script_Registry.h"

namespace Nebula
{

  void ScriptSystem::rebuildFromScene(Scene &scene, ScriptRegistry &registry, ScriptContext &ctx)
  {
    shutdownAll(ctx);
    for (Entity e : scene.getAllEntities())
    {
      if (!scene.hasComponent<ScriptComponent>(e))
        continue;
      const auto &sc = scene.getComponent<ScriptComponent>(e);
      ScriptPtr script = registry.createScript(sc.scriptName);
      if (script)
        m_instances[e.id] = std::move(script);
    }
  }

  void ScriptSystem::initializeAll(ScriptContext &ctx)
  {
    for (auto &[id, script] : m_instances)
    {
      Entity e{id};
      script->onCreate(ctx, e);
      script->onEnable(ctx, e);
    }
  }

  void ScriptSystem::updateAll(ScriptContext &ctx, float dt)
  {
    for (auto &[id, script] : m_instances)
    {
      Entity e{id};
      if (!ctx.scene.isValidEntity(e))
        continue;
      script->onUpdate(ctx, e, dt);
    }
  }

  void ScriptSystem::physicsUpdateAll(ScriptContext &ctx, float fixedDt)
  {
    for (auto &[id, script] : m_instances)
    {
      Entity e{id};
      if (!ctx.scene.isValidEntity(e))
        continue;
      script->onPhysicsUpdate(ctx, e, fixedDt);
    }
  }

  void ScriptSystem::renderAll(ScriptContext &ctx, float dt)
  {
    for (auto &[id, script] : m_instances)
    {
      Entity e{id};
      if (!ctx.scene.isValidEntity(e))
        continue;
      script->onRender(ctx, e, dt);
    }
  }

  void ScriptSystem::shutdownAll(ScriptContext &ctx)
  {
    for (auto &[id, script] : m_instances)
    {
      Entity e{id};
      script->onDisable(ctx, e);
      script->onDestroy(ctx, e);
    }
    m_instances.clear();
  }

}