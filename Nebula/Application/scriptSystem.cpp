#include "scriptSystem.h"
#include "script_Registry.h"
#include <iostream>
#include "component.h"

namespace Nebula
{

  void ScriptSystem::rebuildFromScene(Scene &scene, ScriptRegistry &registry, ScriptContext &ctx)
  {

    shutdownAll(ctx);

    int bindFailures = 0;
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
      else
      {
        bindFailures++;
        std::cerr << "[ScriptSystem] Failed to bind script \""
                  << sc.scriptName
                  << "\" on entity id=" << entity.id;

        if (scene.hasComponent<TagComponent>(entity))
        {
          const auto &tag = scene.getComponent<TagComponent>(entity);
          if (!tag.tag.empty())
          {
            std::cerr << " tag=\"" << tag.tag << '"';
          }
        }

        std::cerr << " — not registered in ScriptRegistry\n";
      }
    }
    if (bindFailures > 0)
    {
      std::cerr << "[ScriptSystem] " << bindFailures
                << " script(s) failed to bind\n";
    }
  }

  void ScriptSystem::bindNewFromScene(Scene &scene, ScriptRegistry &registry, ScriptContext &ctx)
  {
    for (const Entity entity : scene.getAllEntities())
    {
      if (!scene.isValidEntity(entity) || !scene.hasComponent<ScriptComponent>(entity))
      {
        continue;
      }
      if (m_instances.find(entity) != m_instances.end())
      {
        continue;
      }
      const auto &sc = scene.getComponent<ScriptComponent>(entity);
      ScriptPtr script = registry.createScript(sc.scriptName);
      if (script)
      {
        script->onCreate(ctx, entity);
        script->onEnable(ctx, entity);
        m_instances[entity] = std::move(script);
      }
      else
      {
        std::cerr << "[ScriptSystem] Failed to bind new script \""
                  << sc.scriptName << "\" on entity id=" << entity.id << '\n';
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
