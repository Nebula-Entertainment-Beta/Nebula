#pragma once
#include "scene.h" // Entity, component types scripts touch
#include "scene_query.h"
#include "tag_component.h"

namespace Nebula
{

  class ISceneAccess
  {
  public:
    virtual ~ISceneAccess() = default;
    virtual bool isValidEntity(Entity e) const = 0;
    virtual TransformComponent &getTransform(Entity e) = 0;
    virtual CameraComponent &getCamera(Entity e) = 0;
    virtual Entity findByTag(std::string_view tag) = 0;
    virtual const ScriptComponent &getScriptComponent(Entity e) const = 0;
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
    CameraComponent &getCamera(Entity e) override
    {
      return m_scene.getComponent<CameraComponent>(e);
    }
    const ScriptComponent &getScriptComponent(Entity e) const override
    {
      return m_scene.getComponent<ScriptComponent>(e);
    }

    Scene &underlying() { return m_scene; } // engine/internal use only

    Entity findByTag(std::string_view tag) override
    {
      return Nebula::findByTag(m_scene, tag);
    }

  private:
    Scene &m_scene;
  };

} // namespace Nebula