#pragma once
#include "ecs/entity.h"
#include "script.h"
#include "scene.h"
#include <cstdint>
#include <functional>
#include <unordered_map>

namespace Nebula
{
  class ScriptRegistry;

  struct EntityHash
  {
    std::size_t operator()(Entity entity) const noexcept
    {
      const std::size_t h1 = std::hash<EntityID>{}(entity.id);
      const std::size_t h2 = std::hash<uint32_t>{}(entity.generation);
      return h1 ^ (h2 + 0x9e3779b9 + (h1 << 6) + (h1 >> 2));
    }
  };

  class ScriptSystem
  {
  public:
    void rebuildFromScene(Scene &scene, ScriptRegistry &registry, ScriptContext &ctx);
    void initializeAll(ScriptContext &ctx);
    void updateAll(ScriptContext &ctx, float dt);
    void physicsUpdateAll(ScriptContext &ctx, float fixedDt);
    void renderAll(ScriptContext &ctx, float dt);
    void shutdownAll(ScriptContext &ctx);

  private:
    std::unordered_map<Entity, ScriptPtr, EntityHash> m_instances;
  };
} // namespace Nebula
