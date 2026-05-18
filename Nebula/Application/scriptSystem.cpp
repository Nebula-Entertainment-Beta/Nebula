#include "scriptSystem.h"
#include "script_Registry.h"

namespace Nebula
{

  void ScriptSystem::rebuildFromScene(Scene &scene, ScriptRegistry &registry, ScriptContext &ctx)
  {
    shutdownAll(ctx);
    for (const Entity entity : scene.getAllEntities())
    {
      if (!scene.hasComponent<ScriptComponent>(entity))
      {
        continue;
      }
      const auto &sc = scene.getComponent<ScriptComponent>(entity);
      ScriptPtr script = registry.createScript(sc.scriptName);
      if (script)
      {
        m_instances[entity] = std::move(script);
      }
    }
  }

  void ScriptSystem::initializeAll(ScriptContext &ctx)
  {
    for (auto &[entity, script] : m_instances)
    {
      script->onCreate(ctx, entity);
      script->onEnable(ctx, entity);
    }
  }

  void ScriptSystem::updateAll(ScriptContext &ctx, float dt)
  {
    for (auto &[entity, script] : m_instances)
    {
      if (!ctx.scene.isValidEntity(entity))
      {
        continue;
      }
      script->onUpdate(ctx, entity, dt);
    }
  }

  void ScriptSystem::physicsUpdateAll(ScriptContext &ctx, float fixedDt)
  {
    for (auto &[entity, script] : m_instances)
    {
      if (!ctx.scene.isValidEntity(entity))
      {
        continue;
      }
      script->onPhysicsUpdate(ctx, entity, fixedDt);
    }
  }

  void ScriptSystem::renderAll(ScriptContext &ctx, float dt)
  {
    for (auto &[entity, script] : m_instances)
    {
      if (!ctx.scene.isValidEntity(entity))
      {
        continue;
      }
      script->onRender(ctx, entity, dt);
    }
  }

  void ScriptSystem::shutdownAll(ScriptContext &ctx)
  {
    for (auto &[entity, script] : m_instances)
    {
      script->onDisable(ctx, entity);
      script->onDestroy(ctx, entity);
    }
    m_instances.clear();
  }

} // namespace Nebula
