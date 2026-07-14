#include "editorPicking.h"

#include "assetManager.h"
#include "component.h"
#include "mesh_asset.h"
#include "physics/collision_math.h"
#include "physics/collision_types.h"
#include "physics/physics_component.h"
#include "scene.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_inverse.hpp>

#include <algorithm>
#include <cmath>
#include <limits>

namespace Editor
{
  namespace
  {
    glm::vec3 toGlm(const Nebula::Vec3 &v) { return glm::vec3(v.x, v.y, v.z); }
    Nebula::Vec3 fromGlm(const glm::vec3 &v) { return Nebula::Vec3{v.x, v.y, v.z}; }

    glm::mat4 toGlm(const Nebula::Mat4 &m)
    {
      glm::mat4 out;
      for (int c = 0; c < 4; ++c)
      {
        for (int r = 0; r < 4; ++r)
        {
          out[c][r] = m.cols[static_cast<size_t>(c * 4 + r)];
        }
      }
      return out;
    }

    float screenDistance(float ax, float ay, float bx, float by)
    {
      const float dx = ax - bx;
      const float dy = ay - by;
      return std::sqrt(dx * dx + dy * dy);
    }

    bool rayAabbIntersect(const ScreenRay &ray, const Nebula::Vec3 &min, const Nebula::Vec3 &max, float &outT)
    {
      float tMin = 0.0f;
      float tMax = std::numeric_limits<float>::max();
      const float origin[3] = {ray.origin.x, ray.origin.y, ray.origin.z};
      const float dir[3] = {ray.direction.x, ray.direction.y, ray.direction.z};

      for (int axis = 0; axis < 3; ++axis)
      {
        const float mn = (&min.x)[axis];
        const float mx = (&max.x)[axis];
        if (std::fabs(dir[axis]) < 1e-6f)
        {
          if (origin[axis] < mn || origin[axis] > mx)
          {
            return false;
          }
          continue;
        }
        const float ood = 1.0f / dir[axis];
        float t1 = (mn - origin[axis]) * ood;
        float t2 = (mx - origin[axis]) * ood;
        if (t1 > t2)
        {
          std::swap(t1, t2);
        }
        tMin = std::max(tMin, t1);
        tMax = std::min(tMax, t2);
        if (tMax < tMin)
        {
          return false;
        }
      }
      outT = tMin;
      return tMax >= 0.0f;
    }

    bool projectToScreen(const Nebula::Mat4 &viewProjection, const Nebula::Vec3 &world, float viewportWidth,
                         float viewportHeight, float &outX, float &outY)
    {
      const glm::mat4 vp = toGlm(viewProjection);
      const glm::vec4 clip = vp * glm::vec4(toGlm(world), 1.0f);
      if (std::fabs(clip.w) < 1e-6f)
      {
        return false;
      }
      const glm::vec3 ndc = glm::vec3(clip) / clip.w;
      outX = (ndc.x * 0.5f + 0.5f) * viewportWidth;
      outY = (1.0f - (ndc.y * 0.5f + 0.5f)) * viewportHeight;
      return true;
    }

    bool projectToScreen(const Nebula::Camera3D &camera, const Nebula::Vec3 &world, float viewportWidth,
                         float viewportHeight, float &outX, float &outY)
    {
      return projectToScreen(camera.getViewProjectionMatrix(), world, viewportWidth, viewportHeight, outX, outY);
    }
  }

  ScreenRay screenPointToWorldRay(const Nebula::Mat4 &viewProjection, float screenX, float screenY,
                                  float viewportWidth, float viewportHeight)
  {
    const float ndcX = (screenX / viewportWidth) * 2.0f - 1.0f;
    const float ndcY = 1.0f - (screenY / viewportHeight) * 2.0f;

    const glm::mat4 invVp = glm::inverse(toGlm(viewProjection));
    const glm::vec4 nearPoint = invVp * glm::vec4(ndcX, ndcY, -1.0f, 1.0f);
    const glm::vec4 farPoint = invVp * glm::vec4(ndcX, ndcY, 1.0f, 1.0f);
    const glm::vec3 nearWorld = glm::vec3(nearPoint) / nearPoint.w;
    const glm::vec3 farWorld = glm::vec3(farPoint) / farPoint.w;

    ScreenRay ray{};
    ray.origin = fromGlm(nearWorld);
    const glm::vec3 dir = glm::normalize(farWorld - nearWorld);
    ray.direction = fromGlm(dir);
    return ray;
  }

  ScreenRay screenPointToWorldRay(const Nebula::Camera3D &camera, float screenX, float screenY,
                                  float viewportWidth, float viewportHeight)
  {
    return screenPointToWorldRay(camera.getViewProjectionMatrix(), screenX, screenY, viewportWidth,
                                 viewportHeight);
  }

  bool rayPlaneYIntersect(const ScreenRay &ray, float planeY, Nebula::Vec3 &outHit)
  {
    if (std::fabs(ray.direction.y) < 1e-5f)
    {
      return false;
    }
    const float t = (planeY - ray.origin.y) / ray.direction.y;
    if (t < 0.0f)
    {
      return false;
    }
    outHit = {
        ray.origin.x + ray.direction.x * t,
        planeY,
        ray.origin.z + ray.direction.z * t};
    return true;
  }

  Nebula::Entity pickEntity(Nebula::Scene &scene, Nebula::AssetManager &assets, const ScreenRay &ray)
  {
    Nebula::Entity best{};
    float bestT = std::numeric_limits<float>::max();
    Nebula::CollisionMath collisionMath;

    for (const Nebula::Entity entity : scene.getAllEntities())
    {
      if (!scene.hasComponent<Nebula::TransformComponent>(entity))
      {
        continue;
      }

      Nebula::Vec3 min{};
      Nebula::Vec3 max{};
      if (scene.hasComponent<Nebula::ColliderComponent>(entity))
      {
        const Nebula::AABB bounds = collisionMath.worldAABBFromEntity(scene, entity);
        min = bounds.min;
        max = bounds.max;
      }
      else if (scene.hasComponent<Nebula::MeshRendererComponent>(entity))
      {
        const auto &meshRenderer = scene.getComponent<Nebula::MeshRendererComponent>(entity);
        assets.ensureCpuMeshLoaded(meshRenderer.m_meshPath);
        const Nebula::MeshAsset *meshAsset = assets.getCpuMeshAsset(meshRenderer.m_meshPath);
        if (meshAsset == nullptr)
        {
          continue;
        }
        const auto &transform = scene.getComponent<Nebula::TransformComponent>(entity).transform;
        const Nebula::Vec3 pos = transform.getPosition();
        const Nebula::Vec3 scale = transform.getScale();
        min = {
            pos.x + meshAsset->boundsMin.x * scale.x,
            pos.y + meshAsset->boundsMin.y * scale.y,
            pos.z + meshAsset->boundsMin.z * scale.z};
        max = {
            pos.x + meshAsset->boundsMax.x * scale.x,
            pos.y + meshAsset->boundsMax.y * scale.y,
            pos.z + meshAsset->boundsMax.z * scale.z};
      }
      else
      {
        continue;
      }

      float t = 0.0f;
      if (rayAabbIntersect(ray, min, max, t) && t < bestT)
      {
        bestT = t;
        best = entity;
      }
    }
    return best;
  }

  GizmoAxis pickTranslateAxis(const Nebula::Mat4 &viewProjection, const Nebula::Vec3 &origin,
                              float screenX, float screenY, float viewportWidth, float viewportHeight)
  {
    constexpr float kPickRadius = 18.0f;
    const Nebula::Vec3 axes[3] = {
        {origin.x + 1.0f, origin.y, origin.z},
        {origin.x, origin.y + 1.0f, origin.z},
        {origin.x, origin.y, origin.z + 1.0f}};
    const GizmoAxis axisIds[3] = {GizmoAxis::X, GizmoAxis::Y, GizmoAxis::Z};

    GizmoAxis best = GizmoAxis::None;
    float bestDist = kPickRadius;
    for (int i = 0; i < 3; ++i)
    {
      float sx = 0.0f;
      float sy = 0.0f;
      if (!projectToScreen(viewProjection, axes[i], viewportWidth, viewportHeight, sx, sy))
      {
        continue;
      }
      const float dist = screenDistance(screenX, screenY, sx, sy);
      if (dist < bestDist)
      {
        bestDist = dist;
        best = axisIds[i];
      }
    }
    return best;
  }

  GizmoAxis pickTranslateAxis(const Nebula::Camera3D &camera, const Nebula::Vec3 &origin,
                              float screenX, float screenY, float viewportWidth, float viewportHeight)
  {
    return pickTranslateAxis(camera.getViewProjectionMatrix(), origin, screenX, screenY, viewportWidth,
                             viewportHeight);
  }

  bool dragTranslateAxis(GizmoAxis axis, const ScreenRay &ray, const Nebula::Vec3 &dragStartPos,
                         Nebula::Vec3 &outNewPos)
  {
    Nebula::Vec3 planeNormal{};
    switch (axis)
    {
    case GizmoAxis::X:
      planeNormal = {0.0f, 1.0f, 0.0f};
      break;
    case GizmoAxis::Y:
      planeNormal = {1.0f, 0.0f, 0.0f};
      break;
    case GizmoAxis::Z:
      planeNormal = {0.0f, 1.0f, 0.0f};
      break;
    default:
      return false;
    }

    const glm::vec3 n = toGlm(planeNormal);
    const glm::vec3 ro = toGlm(ray.origin);
    const glm::vec3 rd = toGlm(ray.direction);
    const glm::vec3 axisDir = axis == GizmoAxis::X   ? glm::vec3(1, 0, 0)
                            : axis == GizmoAxis::Y ? glm::vec3(0, 1, 0)
                                                   : glm::vec3(0, 0, 1);

    const float denom = glm::dot(rd, n);
    if (std::fabs(denom) < 1e-5f)
    {
      return false;
    }
    const float t = glm::dot(toGlm(dragStartPos) - ro, n) / denom;
    const glm::vec3 hit = ro + rd * t;
    const float along = glm::dot(hit - toGlm(dragStartPos), axisDir);
    outNewPos = {
        dragStartPos.x + axisDir.x * along,
        dragStartPos.y + axisDir.y * along,
        dragStartPos.z + axisDir.z * along};
    return true;
  }

  ColliderHandle pickColliderFace(const Nebula::Mat4 &viewProjection, const Nebula::Vec3 &boxMin,
                                  const Nebula::Vec3 &boxMax, float screenX, float screenY,
                                  float viewportWidth, float viewportHeight)
  {
    constexpr float kPickRadius = 16.0f;
    const Nebula::Vec3 center = {
        (boxMin.x + boxMax.x) * 0.5f,
        (boxMin.y + boxMax.y) * 0.5f,
        (boxMin.z + boxMax.z) * 0.5f};
    const Nebula::Vec3 handles[6] = {
        {boxMax.x, center.y, center.z},
        {boxMin.x, center.y, center.z},
        {center.x, boxMax.y, center.z},
        {center.x, boxMin.y, center.z},
        {center.x, center.y, boxMax.z},
        {center.x, center.y, boxMin.z},
    };
    const ColliderHandle ids[6] = {
        ColliderHandle::PosX, ColliderHandle::NegX, ColliderHandle::PosY,
        ColliderHandle::NegY, ColliderHandle::PosZ, ColliderHandle::NegZ};

    ColliderHandle best = ColliderHandle::None;
    float bestDist = kPickRadius;
    for (int i = 0; i < 6; ++i)
    {
      float sx = 0.0f;
      float sy = 0.0f;
      if (!projectToScreen(viewProjection, handles[i], viewportWidth, viewportHeight, sx, sy))
      {
        continue;
      }
      const float dist = screenDistance(screenX, screenY, sx, sy);
      if (dist < bestDist)
      {
        bestDist = dist;
        best = ids[i];
      }
    }
    return best;
  }

  ColliderHandle pickColliderFace(const Nebula::Camera3D &camera, const Nebula::Vec3 &boxMin,
                                  const Nebula::Vec3 &boxMax, float screenX, float screenY,
                                  float viewportWidth, float viewportHeight)
  {
    return pickColliderFace(camera.getViewProjectionMatrix(), boxMin, boxMax, screenX, screenY,
                            viewportWidth, viewportHeight);
  }

  bool dragColliderFace(ColliderHandle handle, const Nebula::Vec3 &boxCenter,
                        const Nebula::Vec3 &entityScale, bool symmetric, const ScreenRay &ray,
                        Nebula::Vec3 &inOutHalfExtents)
  {
    Nebula::Vec3 hit{};
    if (!rayPlaneYIntersect(ray, boxCenter.y, hit))
    {
      if (handle == ColliderHandle::PosY || handle == ColliderHandle::NegY)
      {
        return false;
      }
      hit = {
          ray.origin.x + ray.direction.x * 4.0f,
          boxCenter.y,
          ray.origin.z + ray.direction.z * 4.0f};
    }

    switch (handle)
    {
    case ColliderHandle::PosX:
    {
      const float scale = std::max(std::fabs(entityScale.x), 0.001f);
      const float worldHalf = std::max(0.01f, std::fabs(hit.x - boxCenter.x));
      inOutHalfExtents.x = worldHalf / scale;
      break;
    }
    case ColliderHandle::NegX:
    {
      const float scale = std::max(std::fabs(entityScale.x), 0.001f);
      const float worldHalf = std::max(0.01f, std::fabs(boxCenter.x - hit.x));
      inOutHalfExtents.x = worldHalf / scale;
      break;
    }
    case ColliderHandle::PosY:
    {
      const float scale = std::max(std::fabs(entityScale.y), 0.001f);
      const float worldHalf = std::max(0.01f, std::fabs(hit.y - boxCenter.y));
      inOutHalfExtents.y = worldHalf / scale;
      break;
    }
    case ColliderHandle::NegY:
    {
      const float scale = std::max(std::fabs(entityScale.y), 0.001f);
      const float worldHalf = std::max(0.01f, std::fabs(boxCenter.y - hit.y));
      inOutHalfExtents.y = worldHalf / scale;
      break;
    }
    case ColliderHandle::PosZ:
    {
      const float scale = std::max(std::fabs(entityScale.z), 0.001f);
      const float worldHalf = std::max(0.01f, std::fabs(hit.z - boxCenter.z));
      inOutHalfExtents.z = worldHalf / scale;
      break;
    }
    case ColliderHandle::NegZ:
    {
      const float scale = std::max(std::fabs(entityScale.z), 0.001f);
      const float worldHalf = std::max(0.01f, std::fabs(boxCenter.z - hit.z));
      inOutHalfExtents.z = worldHalf / scale;
      break;
    }
    default:
      return false;
    }
    return true;
  }

} // namespace Editor
