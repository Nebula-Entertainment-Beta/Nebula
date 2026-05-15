/**
 * @file camera3D.cpp
 * @brief Third-person orbit camera: view + perspective + combined VP.
 */
#include "camera3D.h"

#include <glm/gtc/matrix_transform.hpp>

namespace Nebula {

namespace {
constexpr float kPitchLimit = 1.55f; // ~89° — avoids gimbal singularity with world up

glm::vec3 toGlm(const Vec3 &v)
{
    return glm::vec3(v.x, v.y, v.z);
}

Vec3 fromGlm(const glm::vec3 &v)
{
    return Vec3{v.x, v.y, v.z};
}

Mat4 fromGlm(const glm::mat4 &m)
{
    Mat4 out{};
    for (int c = 0; c < 4; ++c)
    {
        for (int r = 0; r < 4; ++r)
        {
            out.cols[static_cast<std::size_t>(c * 4 + r)] = m[c][r];
        }
    }
    return out;
}

} // namespace

Camera3D::Camera3D() = default;

Vec3 Camera3D::getEyePosition() const
{
    const glm::vec3 target = toGlm(m_target);
    const glm::vec3 pivotOffset = toGlm(m_pivotOffset);
    const glm::vec3 pivot = target + pivotOffset;
    const float pitch = glm::clamp(m_pitch, -kPitchLimit, kPitchLimit);
    const float cosP = glm::cos(pitch);
    const float sinP = glm::sin(pitch);
    const float cosY = glm::cos(m_yaw);
    const float sinY = glm::sin(m_yaw);

    const glm::vec3 offset(
        m_distance * cosP * sinY,
        m_distance * sinP,
        m_distance * cosP * cosY);

    return fromGlm(pivot + offset);
}

Mat4 Camera3D::getViewMatrix() const
{
    const glm::vec3 pivot = toGlm(m_target) + toGlm(m_pivotOffset);
    const glm::vec3 eye = toGlm(getEyePosition());
    return fromGlm(glm::lookAt(eye, pivot, toGlm(m_worldUp)));
}

Mat4 Camera3D::getProjectionMatrix() const
{
    return fromGlm(glm::perspective(glm::radians(m_fov), m_aspectRatio, m_nearPlane, m_farPlane));
}

Mat4 Camera3D::getViewProjectionMatrix() const
{
    const glm::mat4 p = glm::perspective(glm::radians(m_fov), m_aspectRatio, m_nearPlane, m_farPlane);
    const glm::vec3 pivot = toGlm(m_target) + toGlm(m_pivotOffset);
    const glm::vec3 eye = toGlm(getEyePosition());
    const glm::mat4 v = glm::lookAt(eye, pivot, toGlm(m_worldUp));
    return fromGlm(p * v);
}

} // namespace Nebula
