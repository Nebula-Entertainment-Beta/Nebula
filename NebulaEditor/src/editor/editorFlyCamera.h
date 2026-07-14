#pragma once

#include "math_types.h"

namespace Editor
{

  /** First-person fly camera for the Scene View (position + yaw/pitch look). */
  struct EditorFlyCamera
  {
    Nebula::Vec3 position{0.0f, 5.0f, 8.0f};
    float yaw = 0.7f;
    float pitch = -0.3f;
    float fov = 55.0f;
    float nearPlane = 0.1f;
    float farPlane = 200.0f;

    Nebula::Vec3 forward() const;
    Nebula::Vec3 right() const;
    Nebula::Mat4 getViewMatrix() const;
    Nebula::Mat4 getProjectionMatrix(float aspect) const;
    Nebula::Mat4 getViewProjectionMatrix(float aspect) const;

    void moveAlongView(float forwardAmount, float rightAmount, float upAmount);
    void addLookDelta(float yawDelta, float pitchDelta);
  };

} // namespace Editor
