#include "editorTemplate.h"
#include "collider_fit.h"
#include "component.h"
#include <physics/physics_component.h>

namespace Editor
{
  Nebula::Entity EditorTemplate::createEmpty(Nebula::Scene &scene)
  {
    auto entity = scene.createEntity();
    scene.addComponent<Nebula::TransformComponent>(entity);
    auto &t = scene.getComponent<Nebula::TransformComponent>(entity).transform;
    t.setPosition({0.f, 0.5f, 0.f});
    t.setScale(1.f);
    return entity;
  }

  Nebula::Entity EditorTemplate::createMeshCube(Nebula::Scene &scene, const char *tag)
  {
    auto entity = scene.createEntity();
    scene.addComponent<Nebula::TagComponent>(entity).tag = tag;
    auto &mr = scene.addComponent<Nebula::MeshRendererComponent>(entity);
    mr.m_meshPath = "builtin/meshes/cube";
    if (strcmp(tag, "Enemy") == 0)
    {
      mr.m_materialPath = "builtin/materials/enemy";
    }
    else if (strcmp(tag, "Player") == 0)
    {
      mr.m_materialPath = "builtin/materials/player";
    }
    else
    {
      mr.m_materialPath = "builtin/materials/cube";
    }
    scene.addComponent<Nebula::TransformComponent>(entity);
    auto &t = scene.getComponent<Nebula::TransformComponent>(entity).transform;
    t.setPosition({0.f, 0.5f, 0.f});
    t.setScale(1.f);
    return entity;
  }

  Nebula::Entity EditorTemplate::createEnemyPlaceholder(Nebula::Scene &scene)
  {
    auto entity = createMeshCube(scene, "Enemy");

    auto &t = scene.getComponent<Nebula::TransformComponent>(entity).transform;
    t.setPosition({0.f, 0.5f, 0.f});
    t.setScale(1.f);
    return entity;
  }

  Nebula::Entity EditorTemplate::createPlatform(Nebula::Scene &scene, Nebula::AssetManager &assets)
  {
    auto entity = createMeshCube(scene, "Platform");

    auto &t = scene.getComponent<Nebula::TransformComponent>(entity).transform;
    t.setPosition({0.f, 0.5f, 0.f});
    t.setScale(1.f);
    scene.addComponent<Nebula::ColliderComponent>(entity);
    auto &c = scene.getComponent<Nebula::ColliderComponent>(entity);
    c.isStatic = true;
    c.isTrigger = false;
    c.shape = Nebula::ColliderComponent::Shape::Box;
    Nebula::fitBoxColliderToMeshRenderer(c, assets, scene.getComponent<Nebula::MeshRendererComponent>(entity));

    return entity;
  }

  Nebula::Entity EditorTemplate::createBouncePad(Nebula::Scene &scene, Nebula::AssetManager &assets)
  {
    auto entity = createMeshCube(scene, "BouncePad");

    auto &t = scene.getComponent<Nebula::TransformComponent>(entity).transform;
    t.setPosition({0.f, 0.25f, 0.f});
    t.setScale(1.5f);
    scene.addComponent<Nebula::ColliderComponent>(entity);
    auto &c = scene.getComponent<Nebula::ColliderComponent>(entity);
    c.isStatic = true;
    c.isTrigger = true;
    c.shape = Nebula::ColliderComponent::Shape::Box;
    Nebula::fitBoxColliderToMeshRenderer(c, assets, scene.getComponent<Nebula::MeshRendererComponent>(entity));
    scene.addComponent<Nebula::ScriptComponent>(entity).scriptName = "BouncePad";
    return entity;
  }

  Nebula::Entity EditorTemplate::createWindVolume(Nebula::Scene &scene, Nebula::AssetManager &assets)
  {
    auto entity = createMeshCube(scene, "WindVolume");

    auto &t = scene.getComponent<Nebula::TransformComponent>(entity).transform;
    t.setPosition({0.f, 1.f, 0.f});
    t.setScale(2.f);
    scene.addComponent<Nebula::ColliderComponent>(entity);
    auto &c = scene.getComponent<Nebula::ColliderComponent>(entity);
    c.isStatic = true;
    c.isTrigger = true;
    c.shape = Nebula::ColliderComponent::Shape::Box;
    Nebula::fitBoxColliderToMeshRenderer(c, assets, scene.getComponent<Nebula::MeshRendererComponent>(entity));
    scene.addComponent<Nebula::ScriptComponent>(entity).scriptName = "WindVolume";
    return entity;
  }
}
