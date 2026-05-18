#pragma once
#include "scene.h" // Entity, component types scripts touch

namespace Nebula
{

  class ISceneAccess
  {
  public:
    virtual ~ISceneAccess() = default;
    virtual bool isValidEntity(Entity e) const = 0;
    virtual TransformComponent &getTransform(Entity e) = 0;
  };

  class SceneAccess final : public ISceneAccess
  {
  public:
    explicit SceneAccess(Scene &scene) : m_scene(scene) {}
    bool isValidEntity(Entity e) const override { return m_scene.isValidEntity(e); }
    TransformComponent &getTransform(Entity e) override
    {
      return m_scene.getComponent<TransformComponent>(e);
    }

    Scene &underlying() { return m_scene; } // engine/internal use only

  private:
    Scene &m_scene;
  };

} // namespace Nebula