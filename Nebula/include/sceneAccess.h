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
    virtual std::vector<Entity> findAllByTag(std::string_view tag) = 0;
    virtual const ScriptComponent &getScriptComponent(Entity e) const = 0;
    virtual Entity createEntity() = 0;
    virtual void setEntityTag(Entity e, std::string_view tag) = 0;
    virtual void setEntityTransform(Entity e, const Vec3 &position, float yaw, float scale) = 0;
    virtual void setEntityMeshRenderer(Entity e, std::string_view meshPath, std::string_view materialPath) = 0;
    virtual void setEntityScript(Entity e, std::string_view scriptName) = 0;
    virtual void setScriptParamsJson(Entity e, std::string paramsJson) = 0;
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

    Entity createEntity() override
    {
      return m_scene.createEntity();
    }

    void setEntityTag(Entity e, std::string_view tag) override
    {
      m_scene.addComponent<TagComponent>(e).tag = std::string(tag);
    }

    void setEntityTransform(Entity e, const Vec3 &position, float yaw, float scale) override
    {
      auto &transform = m_scene.addComponent<TransformComponent>(e);
      transform.transform.setPosition(position);
      transform.transform.setYaw(yaw);
      transform.transform.setScale(scale);
    }

    void setEntityMeshRenderer(Entity e, std::string_view meshPath, std::string_view materialPath) override
    {
      auto &mesh = m_scene.addComponent<MeshRendererComponent>(e);
      mesh.m_meshPath = std::string(meshPath);
      mesh.m_materialPath = std::string(materialPath);
    }

    void setEntityScript(Entity e, std::string_view scriptName) override
    {
      m_scene.addComponent<ScriptComponent>(e).scriptName = std::string(scriptName);
    }

    void setScriptParamsJson(Entity e, std::string paramsJson) override
    {
      m_scene.getComponent<ScriptComponent>(e).paramsJson = std::move(paramsJson);
    }

    Scene &underlying() { return m_scene; } // engine/internal use only

    Entity findByTag(std::string_view tag) override
    {
      return Nebula::findByTag(m_scene, tag);
    }
    std::vector<Entity> findAllByTag(std::string_view tag) override
    {
      return Nebula::findAllByTag(m_scene, tag);
    }

  private:
    Scene &m_scene;
  };

} // namespace Nebula
