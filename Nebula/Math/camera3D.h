/**
 * @file camera3D.h
 * @brief Third-person perspective camera: orbits a **target** pivot with yaw/pitch and distance.
 *
 * View = `lookAt(eye, pivot, worldUp)` where `eye = pivot + sphericalOffset(distance, yaw, pitch)`.
 * Projection = `glm::perspective` from FOV (degrees), aspect, near, far.
 */
#pragma once

#include <glm/glm.hpp>

namespace Nebula {

class Camera3D {
public:
    Camera3D();

    void setTarget(const glm::vec3& target) { m_target = target; }
    void setPivotOffset(const glm::vec3& offset) { m_pivotOffset = offset; }
    void setDistance(float distance) { m_distance = distance; }
    void setYaw(float yawRadians) { m_yaw = yawRadians; }
    void setPitch(float pitchRadians) { m_pitch = pitchRadians; }
    void setWorldUp(const glm::vec3& up) { m_worldUp = up; }

    void setFOV(float fovDegrees) { m_fov = fovDegrees; }
    void setAspectRatio(float aspect) { m_aspectRatio = aspect; }
    void setNearPlane(float nearPlane) { m_nearPlane = nearPlane; }
    void setFarPlane(float farPlane) { m_farPlane = farPlane; }

    const glm::vec3& getTarget() const { return m_target; }
    const glm::vec3& getPivotOffset() const { return m_pivotOffset; }
    float getDistance() const { return m_distance; }
    float getYaw() const { return m_yaw; }
    float getPitch() const { return m_pitch; }
    const glm::vec3& getWorldUp() const { return m_worldUp; }

    float getFOV() const { return m_fov; }
    float getAspectRatio() const { return m_aspectRatio; }
    float getNearPlane() const { return m_nearPlane; }
    float getFarPlane() const { return m_farPlane; }

    /** Camera world position (orbit point behind pivot). */
    glm::vec3 getEyePosition() const;

    glm::mat4 getViewMatrix() const;
    glm::mat4 getProjectionMatrix() const;
    /** `P * V` for `clip = P * V * M * pos`. */
    glm::mat4 getViewProjectionMatrix() const;

private:
    glm::vec3 m_target{0.0f};
    /** Added to `m_target` to get the look-at pivot (e.g. chest height). */
    glm::vec3 m_pivotOffset{0.0f, 0.35f, 0.0f};
    float m_distance{6.0f};
    float m_yaw{0.0f};
    float m_pitch{-0.25f};
    glm::vec3 m_worldUp{0.0f, 1.0f, 0.0f};

    float m_fov{55.0f};
    float m_aspectRatio{16.0f / 9.0f};
    float m_nearPlane{0.1f};
    float m_farPlane{100.0f};
};

} // namespace Nebula
