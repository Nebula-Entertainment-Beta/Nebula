/**
 * @file camera3D.cpp
 * @brief Third-person orbit camera: view + perspective + combined VP.
 */
#include "camera3D.h"

#include <glm/gtc/matrix_transform.hpp>

namespace Nebula {

namespace {
constexpr float kPitchLimit = 1.55f; // ~89° — avoids gimbal singularity with world up
}

Camera3D::Camera3D() = default;

glm::vec3 Camera3D::getEyePosition() const
{
    const glm::vec3 pivot = m_target + m_pivotOffset;
    const float pitch = glm::clamp(m_pitch, -kPitchLimit, kPitchLimit);
    const float cosP = glm::cos(pitch);
    const float sinP = glm::sin(pitch);
    const float cosY = glm::cos(m_yaw);
    const float sinY = glm::sin(m_yaw);

    // Orbit offset: yaw around world Y, then pitch from horizontal toward +Y
    const glm::vec3 offset(
        m_distance * cosP * sinY,
        m_distance * sinP,
        m_distance * cosP * cosY);

    return pivot + offset;
}

glm::mat4 Camera3D::getViewMatrix() const
{
    const glm::vec3 pivot = m_target + m_pivotOffset;
    const glm::vec3 eye = getEyePosition();
    return glm::lookAt(eye, pivot, m_worldUp);
}

glm::mat4 Camera3D::getProjectionMatrix() const
{
    return glm::perspective(glm::radians(m_fov), m_aspectRatio, m_nearPlane, m_farPlane);
}

glm::mat4 Camera3D::getViewProjectionMatrix() const
{
    return getProjectionMatrix() * getViewMatrix();
}

} // namespace Nebula
