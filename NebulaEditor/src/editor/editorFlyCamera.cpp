#include "editorFlyCamera.h"

#include <glm/gtc/matrix_transform.hpp>

#include <algorithm>
#include <cmath>

namespace Editor
{
  namespace
  {
    constexpr float kPitchLimit = 1.55f;

    glm::vec3 toGlm(const Nebula::Vec3 &v) { return glm::vec3(v.x, v.y, v.z); }
    Nebula::Vec3 fromGlm(const glm::vec3 &v) { return Nebula::Vec3{v.x, v.y, v.z}; }

    Nebula::Mat4 fromGlm(const glm::mat4 &m)
    {
      Nebula::Mat4 out{};
      for (int c = 0; c < 4; ++c)
      {
        for (int r = 0; r < 4; ++r)
        {
          out.cols[static_cast<size_t>(c * 4 + r)] = m[c][r];
        }
      }
      return out;
    }
  }

  Nebula::Vec3 EditorFlyCamera::forward() const
  {
    const float clampedPitch = std::clamp(pitch, -kPitchLimit, kPitchLimit);
    const float cosP = std::cos(clampedPitch);
    const float sinP = std::sin(clampedPitch);
    const float cosY = std::cos(yaw);
    const float sinY = std::sin(yaw);
    return {cosP * sinY, sinP, cosP * cosY};
  }

  Nebula::Vec3 EditorFlyCamera::right() const
  {
    const Nebula::Vec3 f = forward();
    const Nebula::Vec3 up{0.0f, 1.0f, 0.0f};
    const glm::vec3 r = glm::normalize(glm::cross(toGlm(f), toGlm(up)));
    return fromGlm(r);
  }

  Nebula::Mat4 EditorFlyCamera::getViewMatrix() const
  {
    const glm::vec3 eye = toGlm(position);
    const glm::vec3 center = eye + toGlm(forward());
    return fromGlm(glm::lookAt(eye, center, glm::vec3(0.0f, 1.0f, 0.0f)));
  }

  Nebula::Mat4 EditorFlyCamera::getProjectionMatrix(float aspect) const
  {
    return fromGlm(glm::perspective(glm::radians(fov), aspect, nearPlane, farPlane));
  }

  Nebula::Mat4 EditorFlyCamera::getViewProjectionMatrix(float aspect) const
  {
    const glm::mat4 p = glm::perspective(glm::radians(fov), aspect, nearPlane, farPlane);
    const glm::vec3 eye = toGlm(position);
    const glm::vec3 center = eye + toGlm(forward());
    const glm::mat4 v = glm::lookAt(eye, center, glm::vec3(0.0f, 1.0f, 0.0f));
    return fromGlm(p * v);
  }

  void EditorFlyCamera::moveAlongView(float forwardAmount, float rightAmount, float upAmount)
  {
    const Nebula::Vec3 f = forward();
    const Nebula::Vec3 r = right();
    position.x += f.x * forwardAmount + r.x * rightAmount;
    position.y += f.y * forwardAmount + r.y * rightAmount + upAmount;
    position.z += f.z * forwardAmount + r.z * rightAmount;
  }

  void EditorFlyCamera::addLookDelta(float yawDelta, float pitchDelta)
  {
    yaw += yawDelta;
    pitch = std::clamp(pitch + pitchDelta, -kPitchLimit, kPitchLimit);
  }

} // namespace Editor
