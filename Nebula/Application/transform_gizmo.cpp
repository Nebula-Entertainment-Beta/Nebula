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

#include <cmath>
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

    void appendArrowTips(const Vec3 &origin, float length, std::vector<Vec3> &xLines,
                         std::vector<Vec3> &yLines, std::vector<Vec3> &zLines)
    {
      const float tip = length * 0.15f;
      xLines.push_back({origin.x + length, origin.y, origin.z});
      xLines.push_back({origin.x + length - tip, origin.y + tip * 0.5f, origin.z});
      xLines.push_back({origin.x + length, origin.y, origin.z});
      xLines.push_back({origin.x + length - tip, origin.y - tip * 0.5f, origin.z});
      yLines.push_back({origin.x, origin.y + length, origin.z});
      yLines.push_back({origin.x + tip * 0.5f, origin.y + length - tip, origin.z});
      yLines.push_back({origin.x, origin.y + length, origin.z});
      yLines.push_back({origin.x - tip * 0.5f, origin.y + length - tip, origin.z});
      zLines.push_back({origin.x, origin.y, origin.z + length});
      zLines.push_back({origin.x + tip * 0.5f, origin.y, origin.z + length - tip});
      zLines.push_back({origin.x, origin.y, origin.z + length});
      zLines.push_back({origin.x - tip * 0.5f, origin.y, origin.z + length - tip});
    }

    void appendCircle(const Vec3 &origin, float radius, int axis, std::vector<Vec3> &out, int segments = 32)
    {
      constexpr float kPi = 3.14159265f;
      for (int i = 0; i < segments; ++i)
      {
        const float a0 = (kPi * 2.f) * (static_cast<float>(i) / segments);
        const float a1 = (kPi * 2.f) * (static_cast<float>(i + 1) / segments);
        Vec3 p0{};
        Vec3 p1{};
        if (axis == 0) // YZ
        {
          p0 = {origin.x, origin.y + std::cos(a0) * radius, origin.z + std::sin(a0) * radius};
          p1 = {origin.x, origin.y + std::cos(a1) * radius, origin.z + std::sin(a1) * radius};
        }
        else if (axis == 1) // XZ
        {
          p0 = {origin.x + std::cos(a0) * radius, origin.y, origin.z + std::sin(a0) * radius};
          p1 = {origin.x + std::cos(a1) * radius, origin.y, origin.z + std::sin(a1) * radius};
        }
        else // XY
        {
          p0 = {origin.x + std::cos(a0) * radius, origin.y + std::sin(a0) * radius, origin.z};
          p1 = {origin.x + std::cos(a1) * radius, origin.y + std::sin(a1) * radius, origin.z};
        }
        out.push_back(p0);
        out.push_back(p1);
      }
    }

    void appendScaleCubes(const Vec3 &origin, float length, std::vector<Vec3> &xLines,
                          std::vector<Vec3> &yLines, std::vector<Vec3> &zLines)
    {
      const float h = length * 0.08f;
      const auto box = [&](const Vec3 &c, std::vector<Vec3> &lines)
      {
        const Vec3 corners[8] = {
            {c.x - h, c.y - h, c.z - h}, {c.x + h, c.y - h, c.z - h},
            {c.x + h, c.y + h, c.z - h}, {c.x - h, c.y + h, c.z - h},
            {c.x - h, c.y - h, c.z + h}, {c.x + h, c.y - h, c.z + h},
            {c.x + h, c.y + h, c.z + h}, {c.x - h, c.y + h, c.z + h},
        };
        const int edges[12][2] = {{0, 1}, {1, 2}, {2, 3}, {3, 0}, {4, 5}, {5, 6},
                                  {6, 7}, {7, 4}, {0, 4}, {1, 5}, {2, 6}, {3, 7}};
        for (const auto &e : edges)
        {
          lines.push_back(corners[e[0]]);
          lines.push_back(corners[e[1]]);
        }
      };
      box({origin.x + length, origin.y, origin.z}, xLines);
      box({origin.x, origin.y + length, origin.z}, yLines);
      box({origin.x, origin.y, origin.z + length}, zLines);
    }
  } // namespace

  void renderTransformGizmo(const RenderSystemContext &ctx, Entity selectedEntity, int gizmoMode)
  {
    if (!ctx.scene.isValidEntity(selectedEntity) ||
        !ctx.scene.hasComponent<TransformComponent>(selectedEntity))
    {
      return;
    }

    const Entity cameraEntity = findPrimaryCameraEntity(ctx.scene);
    if (cameraEntity.id == 0 && ctx.overrideCamera == nullptr && ctx.overrideViewProjection == nullptr)
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

    const Mat4 vp = [&]() -> Mat4
    {
      if (ctx.overrideViewProjection != nullptr)
      {
        return *ctx.overrideViewProjection;
      }
      if (ctx.overrideCamera != nullptr)
      {
        Camera3D camera = *ctx.overrideCamera;
        camera.setAspectRatio(aspect);
        return camera.getViewProjectionMatrix();
      }

      const Entity camEntity = findPrimaryCameraEntity(ctx.scene);
      if (camEntity.id == 0)
      {
        return Mat4{};
      }

      const auto &cameraComponent = ctx.scene.getComponent<CameraComponent>(camEntity);
      Camera3D camera;
      camera.setTarget(findCameraTarget(ctx.scene, camEntity));
      camera.setPivotOffset(cameraComponent.pivotOffset);
      camera.setDistance(cameraComponent.distance);
      camera.setYaw(cameraComponent.yaw);
      camera.setPitch(cameraComponent.pitch);
      camera.setFOV(cameraComponent.fov);
      camera.setNearPlane(cameraComponent.nearClip);
      camera.setFarPlane(cameraComponent.farClip);
      camera.setAspectRatio(aspect);
      return camera.getViewProjectionMatrix();
    }();

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
    if (gizmoMode == 1)
    {
      appendCircle(origin, axisLength, 0, xLines);
      appendCircle(origin, axisLength, 1, yLines);
      appendCircle(origin, axisLength, 2, zLines);
    }
    else if (gizmoMode == 2)
    {
      appendAxisLines(origin, axisLength, xLines, yLines, zLines);
      appendScaleCubes(origin, axisLength, xLines, yLines, zLines);
    }
    else
    {
      appendAxisLines(origin, axisLength, xLines, yLines, zLines);
      appendArrowTips(origin, axisLength, xLines, yLines, zLines);
    }

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
