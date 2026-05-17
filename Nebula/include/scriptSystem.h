#pragma once
#include "script.h"
#include "scene.h"
#include <unordered_map>
#include <cstdint>

namespace Nebula
{
  class ScriptRegistry;

  class ScriptSystem
  {
  public:
    void rebuildFromScene(Scene &scene, ScriptRegistry &registry, ScriptContext &ctx);
    void initializeAll(ScriptContext &ctx); // or non-const ScriptContext&
    void updateAll(ScriptContext &ctx, float dt);
    void physicsUpdateAll(ScriptContext &ctx, float fixedDt);
    void renderAll(ScriptContext &ctx, float dt);
    void shutdownAll(ScriptContext &ctx);

  private:
    std::unordered_map<EntityID, ScriptPtr> m_instances;
  };
}