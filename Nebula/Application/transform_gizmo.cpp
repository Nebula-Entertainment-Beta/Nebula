#include "transform_gizmo.h"

#include "Window.h"
#include "assetManager.h"
#include "camera3D.h"
#include "component.h"
#include "material.h"
#include "renderer.h"
#include "scene.h"
#include "scene_query.h"
#include "shader.h"

#include <glad/glad.h>

#include <vector>

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
        {
          continue;
        }
        const auto &cam = scene.getComponent<CameraComponent>(entity);
        if (cam.isPrimary)
        {
          return entity;
        }
        if (fallback.id == 0)
        {
          fallback = entity;
        }
      }
      return fallback;
    }

    Vec3 findCameraTarget(Scene &scene, Entity cameraEntity)
    {
      if (!scene.isValidEntity(cameraEntity) || !scene.hasComponent<CameraComponent>(cameraEntity))
      {
        return Vec3{0.f, 0.f, 0.f};
      }
      const CameraComponent &cam = scene.getComponent<CameraComponent>(cameraEntity);
      if (scene.isValidEntity(cam.targetEntity) &&
          scene.hasComponent<TransformComponent>(cam.targetEntity))
      {
        return scene.getComponent<TransformComponent>(cam.targetEntity).transform.getPosition();
      }
      if (!cam.targetTag.empty())
      {
        const Entity tagged = findByTag(scene, cam.targetTag);
        if (scene.isValidEntity(tagged) && scene.hasComponent<TransformComponent>(tagged))
        {
          return scene.getComponent<TransformComponent>(tagged).transform.getPosition();
        }
      }
      return Vec3{0.f, 0.f, 0.f};
    }

    struct DebugLineRenderer
    {
      unsigned int vao = 0;
      unsigned int vbo = 0;
      bool initialized = false;

      void ensureInitialized()
      {
        if (initialized)
        {
          return;
        }
        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);
        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, nullptr);
        glBindVertexArray(0);
        initialized = true;
      }

      void draw(const std::vector<Vec3> &lines, const Mat4 &mvp, Shader &shader, const Vec3 &color)
      {
        if (lines.empty())
        {
          return;
        }

        ensureInitialized();
        std::vector<float> packed;
        packed.reserve(lines.size() * 3);
        for (const Vec3 &p : lines)
        {
          packed.push_back(p.x);
          packed.push_back(p.y);
          packed.push_back(p.z);
        }

        shader.bind();
        shader.setMat4("uMVP", mvp);
        shader.setVec3("uColor", color);
        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(packed.size() * sizeof(float)),
                     packed.data(), GL_DYNAMIC_DRAW);
        glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(lines.size()));
        glBindVertexArray(0);
        shader.unbind();
      }
    };

    void appendAxisLines(const Vec3 &origin, float length, std::vector<Vec3> &xLines,
                         std::vector<Vec3> &yLines, std::vector<Vec3> &zLines)
    {
      xLines.push_back(origin);
      xLines.push_back({origin.x + length, origin.y, origin.z});
      yLines.push_back(origin);
      yLines.push_back({origin.x, origin.y + length, origin.z});
      zLines.push_back(origin);
      zLines.push_back({origin.x, origin.y, origin.z + length});
    }
  } // namespace

  void renderTransformGizmo(const RenderSystemContext &ctx, Entity selectedEntity)
  {
    if (!ctx.scene.isValidEntity(selectedEntity) ||
        !ctx.scene.hasComponent<TransformComponent>(selectedEntity))
    {
      return;
    }

    const Entity cameraEntity = findPrimaryCameraEntity(ctx.scene);
    if (cameraEntity.id == 0)
    {
      return;
    }

    int fbw = 0;
    int fbh = 0;
    if (ctx.viewportWidth != 0 && ctx.viewportHeight != 0)
    {
      fbw = static_cast<int>(ctx.viewportWidth);
      fbh = static_cast<int>(ctx.viewportHeight);
    }
    else
    {
      ctx.window.getFramebufferSize(fbw, fbh);
    }
    const float aspect = (fbh > 0) ? (static_cast<float>(fbw) / static_cast<float>(fbh)) : (16.0f / 9.0f);

    const auto &cameraComponent = ctx.scene.getComponent<CameraComponent>(cameraEntity);
    Camera3D camera;
    camera.setTarget(findCameraTarget(ctx.scene, cameraEntity));
    camera.setPivotOffset(cameraComponent.pivotOffset);
    camera.setDistance(cameraComponent.distance);
    camera.setYaw(cameraComponent.yaw);
    camera.setPitch(cameraComponent.pitch);
    camera.setFOV(cameraComponent.fov);
    camera.setNearPlane(cameraComponent.nearClip);
    camera.setFarPlane(cameraComponent.farClip);
    camera.setAspectRatio(aspect);

    const Mat4 vp = camera.getViewProjectionMatrix();
    const Material *debugMaterial = ctx.assets.getMaterial(
        ctx.assets.loadMaterial("builtin/materials/cube", ctx.renderer.resources()));
    if (debugMaterial == nullptr || debugMaterial->shader == nullptr)
    {
      return;
    }

    const Vec3 origin = ctx.scene.getComponent<TransformComponent>(selectedEntity).transform.getPosition();
    const float axisLength = 1.0f;

    static DebugLineRenderer lineRenderer;
    std::vector<Vec3> xLines;
    std::vector<Vec3> yLines;
    std::vector<Vec3> zLines;
    appendAxisLines(origin, axisLength, xLines, yLines, zLines);

    GLboolean depthWasEnabled = glIsEnabled(GL_DEPTH_TEST);
    glEnable(GL_DEPTH_TEST);
    glLineWidth(3.f);

    lineRenderer.draw(xLines, vp, *debugMaterial->shader, Vec3{0.95f, 0.25f, 0.25f});
    lineRenderer.draw(yLines, vp, *debugMaterial->shader, Vec3{0.25f, 0.95f, 0.25f});
    lineRenderer.draw(zLines, vp, *debugMaterial->shader, Vec3{0.25f, 0.45f, 0.95f});

    glLineWidth(1.f);
    if (!depthWasEnabled)
    {
      glDisable(GL_DEPTH_TEST);
    }
  }

} // namespace Nebula
