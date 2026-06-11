#include "sceneDefaults.h"

#include "component.h"
#include "nimbus_config.h"
#include "tag_component.h"

namespace Nimbus
{

  void buildDefaultScene(Nebula::Scene &scene)
  {
    const Nebula::Entity groundEntity = scene.createEntity();
    scene.addComponent<Nebula::TagComponent>(groundEntity).tag = kGroundTag;
    auto &groundTransform = scene.addComponent<Nebula::TransformComponent>(groundEntity);
    groundTransform.transform.setPosition(Nebula::Vec3{0.0f, 0.0f, 0.0f});
    groundTransform.transform.setYaw(0.0f);
    groundTransform.transform.setScale(20.0f);
    auto &groundMesh = scene.addComponent<Nebula::MeshRendererComponent>(groundEntity);
    groundMesh.m_meshPath = "builtin/meshes/ground";
    groundMesh.m_materialPath = "builtin/materials/ground";
    scene.addComponent<Nebula::ScriptComponent>(groundEntity).scriptName = "Ground";

    const Nebula::Entity cubeEntity = scene.createEntity();
    scene.addComponent<Nebula::TagComponent>(cubeEntity).tag = kPlayerTag;
    auto &cubeTransform = scene.addComponent<Nebula::TransformComponent>(cubeEntity);
    cubeTransform.transform.setPosition(Nebula::Vec3{0.0f, 0.5f, 0.0f});
    cubeTransform.transform.setYaw(0.0f);
    cubeTransform.transform.setScale(1.0f);
    auto &cubeMesh = scene.addComponent<Nebula::MeshRendererComponent>(cubeEntity);
    cubeMesh.m_meshPath = "builtin/meshes/cube";
    cubeMesh.m_materialPath = "builtin/materials/player";
    scene.addComponent<Nebula::ScriptComponent>(cubeEntity).scriptName = "Player";

    const Nebula::Entity cameraEntity = scene.createEntity();
    scene.addComponent<Nebula::TagComponent>(cameraEntity).tag = kMainCameraTag;
    scene.addComponent<Nebula::TransformComponent>(cameraEntity);
    auto &cameraComponent = scene.addComponent<Nebula::CameraComponent>(cameraEntity);
    cameraComponent.pivotOffset = Nebula::Vec3{0.0f, 0.35f, 0.0f};
    cameraComponent.distance = 6.0f;
    cameraComponent.yaw = 0.7f;
    cameraComponent.pitch = -0.3f;
    cameraComponent.fov = 55.0f;
    cameraComponent.nearClip = 0.1f;
    cameraComponent.farClip = 100.0f;
    cameraComponent.targetTag = kPlayerTag;
    cameraComponent.targetEntity = cubeEntity;
    scene.addComponent<Nebula::ScriptComponent>(cameraEntity).scriptName = "MainCamera";

    const Nebula::Entity directorEntity = scene.createEntity();
    scene.addComponent<Nebula::TagComponent>(directorEntity).tag = "CombatDirector";
    scene.addComponent<Nebula::TransformComponent>(directorEntity);
    scene.addComponent<Nebula::ScriptComponent>(directorEntity).scriptName = "CombatDirector";
  }

}
