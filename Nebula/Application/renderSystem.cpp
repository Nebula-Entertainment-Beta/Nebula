#include "renderSystem.h"
#include "assetManager.h"
#include "mesh.h"
#include "material.h"
#include "texture.h"
#include "component.h"
#include "environment.h"
#include "scene.h"
#include "camera3D.h"
#include "renderer.h"
#include "Window.h"
#include "scene_query.h"

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

    Vec3 fallbackTarget(Scene &scene, Entity cameraEntity)
    {
      for (const Entity entity : scene.getAllEntities())
      {
        if (entity == cameraEntity)
          continue;
        if (scene.hasComponent<CameraComponent>(entity))
          continue;
        if (scene.hasComponent<TransformComponent>(entity) &&
            scene.hasComponent<MeshRendererComponent>(entity))
        {
          return scene.getComponent<TransformComponent>(entity).transform.getPosition();
        }
      }
      return Vec3{0.0f, 0.0f, 0.0f};
    }

    Vec3 findCameraTarget(Scene &scene, Entity cameraEntity)
    {
      if (!scene.isValidEntity(cameraEntity) ||
          !scene.hasComponent<CameraComponent>(cameraEntity))
      {
        return fallbackTarget(scene, cameraEntity);
      }

      const CameraComponent &cam = scene.getComponent<CameraComponent>(cameraEntity);

      // --- B: from camera data (entity reference) ---
      if (scene.isValidEntity(cam.targetEntity) &&
          scene.hasComponent<TransformComponent>(cam.targetEntity))
      {
        return scene.getComponent<TransformComponent>(cam.targetEntity)
            .transform.getPosition();
      }

      // --- C: FollowTarget component on the camera entity ---
      if (scene.hasComponent<followTargetComponent>(cameraEntity))
      {
        const followTargetComponent &ft = scene.getComponent<followTargetComponent>(cameraEntity);
        if (scene.isValidEntity(ft.targetEntity) &&
            scene.hasComponent<TransformComponent>(ft.targetEntity))
        {
          return scene.getComponent<TransformComponent>(ft.targetEntity)
              .transform.getPosition();
        }
        if (!ft.targetTag.empty())
        {
          const Entity tagged = findByTag(scene, ft.targetTag);
          if (scene.isValidEntity(tagged) && scene.hasComponent<TransformComponent>(tagged))
          {
            return scene.getComponent<TransformComponent>(tagged).transform.getPosition();
          }
        }
      }

      // --- A: tag on camera data ---
      if (!cam.targetTag.empty())
      {
        const Entity tagged = findByTag(scene, cam.targetTag);
        if (scene.isValidEntity(tagged) && scene.hasComponent<TransformComponent>(tagged))
        {
          return scene.getComponent<TransformComponent>(tagged).transform.getPosition();
        }
      }

      return fallbackTarget(scene, cameraEntity);
    }
  } // namespace

  void renderScene(const RenderSystemContext &ctx)
  {
    const Entity cameraEntity = findPrimaryCameraEntity(ctx.scene);
    if (cameraEntity.id == 0 && ctx.overrideCamera == nullptr && ctx.overrideViewProjection == nullptr)
      return;

    int fbw = 0, fbh = 0;
    if (ctx.viewportWidth != 0 && ctx.viewportHeight != 0)
    {
      fbw = static_cast<int>(ctx.viewportWidth);
      fbh = static_cast<int>(ctx.viewportHeight);
      ctx.renderer.setViewport(static_cast<int>(ctx.viewportX), static_cast<int>(ctx.viewportY), ctx.viewportWidth, ctx.viewportHeight);
    }
    else
    {

      ctx.window.getFramebufferSize(fbw, fbh);
    }
    const float aspect = (fbh > 0)
                             ? (static_cast<float>(fbw) / static_cast<float>(fbh))
                             : (16.0f / 9.0f);

    Camera3D activeCamera;
    Vec3 cameraEye{};
    const Mat4 vp = [&]() -> Mat4
    {
      if (ctx.overrideViewProjection != nullptr)
      {
        if (ctx.overrideCamera != nullptr)
        {
          cameraEye = ctx.overrideCamera->getEyePosition();
        }
        return *ctx.overrideViewProjection;
      }
      if (ctx.overrideCamera != nullptr)
      {
        activeCamera = *ctx.overrideCamera;
        activeCamera.setAspectRatio(aspect);
        cameraEye = activeCamera.getEyePosition();
        return activeCamera.getViewProjectionMatrix();
      }

      const auto &cameraComponent = ctx.scene.getComponent<CameraComponent>(cameraEntity);
      activeCamera.setTarget(findCameraTarget(ctx.scene, cameraEntity));
      activeCamera.setPivotOffset(cameraComponent.pivotOffset);
      activeCamera.setDistance(cameraComponent.distance);
      activeCamera.setYaw(cameraComponent.yaw);
      activeCamera.setPitch(cameraComponent.pitch);
      activeCamera.setFOV(cameraComponent.fov);
      activeCamera.setNearPlane(cameraComponent.nearClip);
      activeCamera.setFarPlane(cameraComponent.farClip);
      activeCamera.setAspectRatio(aspect);
      cameraEye = activeCamera.getEyePosition();
      return activeCamera.getViewProjectionMatrix();
    }();

    const EnvironmentComponent env = findEnvironmentOrDefault(ctx.scene);

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
      material->shader->setVec3("uLightDir", env.lightDirection);
      material->shader->setVec3("uLightColor", env.lightColor);
      material->shader->setFloat("uLightIntensity", env.lightIntensity);
      material->shader->setVec3("uFogColor", env.fogColor);
      material->shader->setFloat("uFogDensity", env.fogDensity);
      material->shader->setVec3("uCameraPos", cameraEye);
      if (material->albedoTexture)
      {
        material->shader->setInt("uUseTexture", 1);
        material->shader->setInt("uAlbedoMap", 0);
        material->albedoTexture->bind(0);
      }
      else
      {
        material->shader->setInt("uUseTexture", 0);
      }
      ctx.renderer.drawIndexed(mesh->vao, mesh->indexCount);
      if (material->albedoTexture)
      {
        material->albedoTexture->unbind(0);
      }
      material->shader->unbind();
    }
  }

} // namespace Nebula