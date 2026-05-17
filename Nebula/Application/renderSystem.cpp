#include "renderSystem.h"
#include "assetManager.h"
#include "mesh.h"
#include "material.h"
#include "component.h"
#include "scene.h"
#include "camera3D.h"
#include "renderer.h"
#include "Window.h"

#include <cmath>

namespace Nebula
{
  namespace
  {
    Entity findPrimaryCameraEntity(Scene &scene)
    {
      Entity fallback{};
      for (const Entity entity : scene.getAllEntities())
      {
        if (!scene.hasComponent<CameraComponent>(entity))
          continue;
        const auto &cam = scene.getComponent<CameraComponent>(entity);
        if (cam.isPrimary)
          return entity;
        if (fallback.id == 0)
          fallback = entity;
      }
      return fallback;
    }

    Vec3 findCameraTarget(Scene &scene)
    {
      for (const Entity entity : scene.getAllEntities())
      {
        if (!scene.hasComponent<ScriptComponent>(entity))
          continue;
        if (scene.getComponent<ScriptComponent>(entity).scriptName != "Player")
          continue;
        if (scene.hasComponent<TransformComponent>(entity))
          return scene.getComponent<TransformComponent>(entity).transform.getPosition();
      }
      for (const Entity entity : scene.getAllEntities())
      {
        if (scene.hasComponent<TransformComponent>(entity) &&
            scene.hasComponent<MeshRendererComponent>(entity))
          return scene.getComponent<TransformComponent>(entity).transform.getPosition();
      }
      return Vec3{0.0f, 0.0f, 0.0f};
    }
  } // namespace

  void renderScene(const RenderSystemContext &ctx)
  {
    const Entity cameraEntity = findPrimaryCameraEntity(ctx.scene);
    if (cameraEntity.id == 0)
      return;

    int fbw = 0, fbh = 0;
    ctx.window.getFramebufferSize(fbw, fbh);
    const float aspect = (fbh > 0)
                             ? (static_cast<float>(fbw) / static_cast<float>(fbh))
                             : (16.0f / 9.0f);

    const auto &cameraComponent = ctx.scene.getComponent<CameraComponent>(cameraEntity);
    Camera3D camera;
    camera.setTarget(findCameraTarget(ctx.scene));
    camera.setPivotOffset(cameraComponent.pivotOffset);
    camera.setDistance(cameraComponent.distance);
    camera.setYaw(cameraComponent.yaw);
    camera.setPitch(cameraComponent.pitch);
    camera.setFOV(cameraComponent.fov);
    camera.setNearPlane(cameraComponent.nearClip);
    camera.setFarPlane(cameraComponent.farClip);
    camera.setAspectRatio(aspect);

    const Mat4 vp = camera.getViewProjectionMatrix();

    for (const Entity entity : ctx.scene.getAllEntities())
    {
      if (!ctx.scene.hasComponent<TransformComponent>(entity) ||
          !ctx.scene.hasComponent<MeshRendererComponent>(entity))
        continue;

      const auto &transform = ctx.scene.getComponent<TransformComponent>(entity);
      const auto &meshRenderer = ctx.scene.getComponent<MeshRendererComponent>(entity);

      const Mesh *mesh = ctx.assets.getMesh(meshRenderer.m_meshID);
      const Material *material = ctx.assets.getMaterial(meshRenderer.m_materialID);
      if (!mesh || !material || !material->shader)
        continue;

      material->shader->bind();
      const Mat4 mvp = vp * transform.transform.getModelMatrix();
      material->shader->setMat4("uMVP", mvp);
      material->shader->setVec3("uColor", material->color);
      ctx.renderer.drawIndexed(mesh->vao, mesh->indexCount);
      material->shader->unbind();
    }
  }

} // namespace Nebula