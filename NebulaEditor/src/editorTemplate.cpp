#include "editorTemplate.h"

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
    mr.m_materialPath = "builtin/materials/cube";
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
}