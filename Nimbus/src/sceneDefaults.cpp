#include "sceneDefaults.h"

#include "component.h"
#include "nimbus_config.h"
#include "physics/physics_component.h"
#include "tag_component.h"

namespace Nimbus
{

  namespace
  {

    Nebula::Entity addSolidPlatform(Nebula::Scene &scene, const Nebula::Vec3 &position, float scale,
                                     const char *tag)
    {
      const Nebula::Entity entity = scene.createEntity();
      scene.addComponent<Nebula::TagComponent>(entity).tag = tag;
      auto &transform = scene.addComponent<Nebula::TransformComponent>(entity);
      transform.transform.setPosition(position);
      transform.transform.setYaw(0.0f);
      transform.transform.setScale(scale);
      auto &mesh = scene.addComponent<Nebula::MeshRendererComponent>(entity);
      mesh.m_meshPath = "builtin/meshes/cube";
      mesh.m_materialPath = "builtin/materials/cube";
      auto &collider = scene.addComponent<Nebula::ColliderComponent>(entity);
      collider.halfExtents = {0.5f, 0.5f, 0.5f};
      collider.isStatic = true;
      collider.isTrigger = false;
      collider.shape = Nebula::ColliderComponent::Shape::Box;
      return entity;
    }

    Nebula::Entity addBouncePad(Nebula::Scene &scene, const Nebula::Vec3 &position, float scale)
    {
      const Nebula::Entity entity = scene.createEntity();
      scene.addComponent<Nebula::TagComponent>(entity).tag = kBouncePadTag;
      auto &transform = scene.addComponent<Nebula::TransformComponent>(entity);
      transform.transform.setPosition(position);
      transform.transform.setYaw(0.0f);
      transform.transform.setScale(scale);
      auto &mesh = scene.addComponent<Nebula::MeshRendererComponent>(entity);
      mesh.m_meshPath = "meshes/bounce_pad.mesh";
      mesh.m_materialPath = "materials/bounce_pad.mat";
      auto &collider = scene.addComponent<Nebula::ColliderComponent>(entity);
      collider.halfExtents = {0.5f, 0.5f, 0.5f};
      collider.isStatic = true;
      collider.isTrigger = true;
      collider.shape = Nebula::ColliderComponent::Shape::Box;
      scene.addComponent<Nebula::ScriptComponent>(entity).scriptName = "BouncePad";
      return entity;
    }

    Nebula::Entity addWindVolume(Nebula::Scene &scene, const Nebula::Vec3 &position, float scale)
    {
      const Nebula::Entity entity = scene.createEntity();
      scene.addComponent<Nebula::TagComponent>(entity).tag = kWindVolumeTag;
      auto &transform = scene.addComponent<Nebula::TransformComponent>(entity);
      transform.transform.setPosition(position);
      transform.transform.setYaw(0.0f);
      transform.transform.setScale(scale);
      auto &mesh = scene.addComponent<Nebula::MeshRendererComponent>(entity);
      mesh.m_meshPath = "meshes/wind_volume.mesh";
      mesh.m_materialPath = "materials/wind_volume.mat";
      auto &collider = scene.addComponent<Nebula::ColliderComponent>(entity);
      collider.halfExtents = {0.5f, 0.5f, 0.5f};
      collider.isStatic = true;
      collider.isTrigger = true;
      collider.shape = Nebula::ColliderComponent::Shape::Box;
      scene.addComponent<Nebula::ScriptComponent>(entity).scriptName = "WindVolume";
      return entity;
    }

    Nebula::Entity addPlayer(Nebula::Scene &scene, const Nebula::Vec3 &position)
    {
      const Nebula::Entity entity = scene.createEntity();
      scene.addComponent<Nebula::TagComponent>(entity).tag = kPlayerTag;
      auto &transform = scene.addComponent<Nebula::TransformComponent>(entity);
      transform.transform.setPosition(position);
      transform.transform.setYaw(0.0f);
      transform.transform.setScale(1.0f);
      auto &mesh = scene.addComponent<Nebula::MeshRendererComponent>(entity);
      mesh.m_meshPath = "builtin/meshes/cube";
      mesh.m_materialPath = "builtin/materials/player";
      auto &collider = scene.addComponent<Nebula::ColliderComponent>(entity);
      collider.halfExtents = {0.5f, 0.5f, 0.5f};
      collider.isStatic = false;
      collider.isTrigger = false;
      collider.shape = Nebula::ColliderComponent::Shape::Box;
      auto &body = scene.addComponent<Nebula::RigidBodyComponent>(entity);
      body.kinematic = true;
      body.mass = 1.f;
      scene.addComponent<Nebula::ScriptComponent>(entity).scriptName = "Player";
      return entity;
    }

    Nebula::Entity addMainCamera(Nebula::Scene &scene, Nebula::Entity playerEntity)
    {
      const Nebula::Entity entity = scene.createEntity();
      scene.addComponent<Nebula::TagComponent>(entity).tag = kMainCameraTag;
      scene.addComponent<Nebula::TransformComponent>(entity);
      auto &cameraComponent = scene.addComponent<Nebula::CameraComponent>(entity);
      cameraComponent.pivotOffset = Nebula::Vec3{0.0f, 0.35f, 0.0f};
      cameraComponent.distance = 6.0f;
      cameraComponent.yaw = 0.7f;
      cameraComponent.pitch = -0.3f;
      cameraComponent.fov = 55.0f;
      cameraComponent.nearClip = 0.1f;
      cameraComponent.farClip = 100.0f;
      cameraComponent.targetTag = kPlayerTag;
      cameraComponent.targetEntity = playerEntity;
      scene.addComponent<Nebula::ScriptComponent>(entity).scriptName = "MainCamera";
      return entity;
    }

    float platformTopY(float centerY, float scale)
    {
      return centerY + 0.5f * scale;
    }

    float playerSpawnYOnPlatform(float platformCenterY, float platformScale, float playerScale = 1.0f)
    {
      const float platformHalfHeight = 0.5f * platformScale;
      const float playerHalfHeight = 0.5f * playerScale;
      return platformCenterY + platformHalfHeight + playerHalfHeight + 0.5f;
    }

  } // namespace

  void buildDefaultScene(Nebula::Scene &scene)
  {
    constexpr float kGroundScale = 20.0f;
    constexpr float kGroundWorldHalfY = 0.05f;

    const Nebula::Entity groundEntity = scene.createEntity();
    scene.addComponent<Nebula::TagComponent>(groundEntity).tag = kGroundTag;
    auto &groundTransform = scene.addComponent<Nebula::TransformComponent>(groundEntity);
    // Thin collider slab: top face at y=0 (mesh plane). Transform sits below so scaled
    // halfExtents align with the visible ground surface, not a meter above it.
    groundTransform.transform.setPosition(Nebula::Vec3{0.0f, 9.73f, 0.0f});
    groundTransform.transform.setYaw(0.0f);
    groundTransform.transform.setScale(0.12f);
    auto &groundMesh = scene.addComponent<Nebula::MeshRendererComponent>(groundEntity);
    groundMesh.m_meshPath = "meshes/ground_cloud.mesh";
    groundMesh.m_materialPath = "materials/ground_cloud.mat";
    auto &groundCollider = scene.addComponent<Nebula::ColliderComponent>(groundEntity);
    groundCollider.halfExtents = {12.0f, kGroundWorldHalfY / kGroundScale, 12.0f};
    groundCollider.isStatic = true;
    groundCollider.isTrigger = false;
    groundCollider.shape = Nebula::ColliderComponent::Shape::Box;
    scene.addComponent<Nebula::ScriptComponent>(groundEntity).scriptName = "Ground";

    const Nebula::Entity platformEntity = scene.createEntity();
    scene.addComponent<Nebula::TagComponent>(platformEntity).tag = "Platform";

    auto &platformTransform = scene.addComponent<Nebula::TransformComponent>(platformEntity);
    platformTransform.transform.setPosition(Nebula::Vec3{3.0f, 1.0f, 0.0f});
    platformTransform.transform.setYaw(0.0f);
    platformTransform.transform.setScale(2.0f);

    auto &platformMesh = scene.addComponent<Nebula::MeshRendererComponent>(platformEntity);
    platformMesh.m_meshPath = "builtin/meshes/cube";
    platformMesh.m_materialPath = "builtin/materials/cube";

    auto &platformCollider = scene.addComponent<Nebula::ColliderComponent>(platformEntity);
    platformCollider.halfExtents = {0.5f, 0.5f, 0.5f}; // local; scale 2 => ~1m tall slab
    platformCollider.isStatic = true;
    platformCollider.isTrigger = false;
    platformCollider.shape = Nebula::ColliderComponent::Shape::Box;

    const Nebula::Entity cubeEntity = scene.createEntity();
    scene.addComponent<Nebula::TagComponent>(cubeEntity).tag = kPlayerTag;
    auto &cubeTransform = scene.addComponent<Nebula::TransformComponent>(cubeEntity);
    cubeTransform.transform.setPosition(Nebula::Vec3{0.0f, 0.5f, 0.0f});
    cubeTransform.transform.setYaw(0.0f);
    cubeTransform.transform.setScale(1.0f);
    auto &cubeMesh = scene.addComponent<Nebula::MeshRendererComponent>(cubeEntity);
    cubeMesh.m_meshPath = "builtin/meshes/cube";
    cubeMesh.m_materialPath = "builtin/materials/player";
    auto &cubeCollider = scene.addComponent<Nebula::ColliderComponent>(cubeEntity);
    cubeCollider.halfExtents = {0.5f, 0.5f, 0.5f};
    cubeCollider.isStatic = false;
    cubeCollider.isTrigger = false;
    cubeCollider.shape = Nebula::ColliderComponent::Shape::Box;
    auto &body = scene.addComponent<Nebula::RigidBodyComponent>(cubeEntity);
    body.kinematic = true;
    body.mass = 1.f;
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

  void buildCombatArenaScene(Nebula::Scene &scene)
  {
    buildDefaultScene(scene);
  }

  void buildTraversalTestScene(Nebula::Scene &scene)
  {
    constexpr float kSpawnPlatformScale = 4.0f;
    const Nebula::Vec3 kSpawnPlatformCenter{0.0f, 0.0f, 0.0f};

    addSolidPlatform(scene, kSpawnPlatformCenter, kSpawnPlatformScale, kSpawnTag);
    addSolidPlatform(scene, Nebula::Vec3{7.0f, 1.0f, 0.0f}, 2.0f, "Platform");
    addSolidPlatform(scene, Nebula::Vec3{4.5f, 0.0f, 0.0f}, 1.5f, "Platform");
    addBouncePad(scene, Nebula::Vec3{4.5f, 0.26f, 0.0f}, 1.2f);
    addSolidPlatform(scene, Nebula::Vec3{11.0f, 4.0f, 0.0f}, 2.0f, "Platform");
    addWindVolume(scene, Nebula::Vec3{13.5f, 6.0f, 0.0f}, 1.5f);
    addSolidPlatform(scene, Nebula::Vec3{16.0f, 7.5f, 0.0f}, 2.0f, kGoalTag);

    const float spawnY = playerSpawnYOnPlatform(kSpawnPlatformCenter.y, kSpawnPlatformScale);
    const Nebula::Entity playerEntity =
        addPlayer(scene, Nebula::Vec3{kSpawnPlatformCenter.x, spawnY, kSpawnPlatformCenter.z});
    addMainCamera(scene, playerEntity);

    const Nebula::Entity directorEntity = scene.createEntity();
    scene.addComponent<Nebula::TagComponent>(directorEntity).tag = "TraversalDirector";
    scene.addComponent<Nebula::TransformComponent>(directorEntity);
    scene.addComponent<Nebula::ScriptComponent>(directorEntity).scriptName = "TraversalDirector";
  }

}
