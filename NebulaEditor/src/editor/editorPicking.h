#pragma once

#include "camera3D.h"
#include "ecs/entity.h"
#include "math_types.h"

namespace Nebula
{
  class AssetManager;
  class Scene;
}

namespace Editor
{

  struct ScreenRay
  {
    Nebula::Vec3 origin{};
    Nebula::Vec3 direction{};
  };

  ScreenRay screenPointToWorldRay(const Nebula::Camera3D &camera, float screenX, float screenY,
                                  float viewportWidth, float viewportHeight);
  ScreenRay screenPointToWorldRay(const Nebula::Mat4 &viewProjection, float screenX, float screenY,
                                  float viewportWidth, float viewportHeight);

  bool rayPlaneYIntersect(const ScreenRay &ray, float planeY, Nebula::Vec3 &outHit);

  Nebula::Entity pickEntity(Nebula::Scene &scene, Nebula::AssetManager &assets, const ScreenRay &ray);

  enum class GizmoAxis
  {
    None,
    X,
    Y,
    Z
  };

  GizmoAxis pickTranslateAxis(const Nebula::Camera3D &camera, const Nebula::Vec3 &origin,
                              float screenX, float screenY, float viewportWidth, float viewportHeight,
                              float axisLength = 1.f);
  GizmoAxis pickTranslateAxis(const Nebula::Mat4 &viewProjection, const Nebula::Vec3 &origin,
                              float screenX, float screenY, float viewportWidth, float viewportHeight,
                              float axisLength = 1.f);

  bool dragTranslateAxis(GizmoAxis axis, const ScreenRay &ray, const Nebula::Vec3 &dragStartPos,
                         Nebula::Vec3 &outNewPos);

  enum class ColliderHandle
  {
    None,
    PosX,
    NegX,
    PosY,
    NegY,
    PosZ,
    NegZ
  };

  ColliderHandle pickColliderFace(const Nebula::Camera3D &camera, const Nebula::Vec3 &boxMin,
                                  const Nebula::Vec3 &boxMax, float screenX, float screenY,
                                  float viewportWidth, float viewportHeight);
  ColliderHandle pickColliderFace(const Nebula::Mat4 &viewProjection, const Nebula::Vec3 &boxMin,
                                  const Nebula::Vec3 &boxMax, float screenX, float screenY,
                                  float viewportWidth, float viewportHeight);

  bool dragColliderFace(ColliderHandle handle, const Nebula::Vec3 &boxCenter,
                        const Nebula::Vec3 &entityScale, bool symmetric, const ScreenRay &ray,
                        Nebula::Vec3 &inOutHalfExtents);

} // namespace Editor
