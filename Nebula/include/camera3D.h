/**
 * @file camera3D.h
 * @brief Third-person perspective camera: orbits a **target** pivot with yaw/pitch and distance.
 *
 * View = `lookAt(eye, pivot, worldUp)`; projection = perspective from FOV (degrees), aspect, near, far.
 * Implementation uses glm internally; the public surface uses `Vec3` / `Mat4` only.
 */
#pragma once

#include "math_types.h"

namespace Nebula
{

class Camera3D
{
public:
    Camera3D();

    void setTarget(const Vec3 &target) { m_target = target; }
    void setPivotOffset(const Vec3 &offset) { m_pivotOffset = offset; }
    void setDistance(float distance) { m_distance = distance; }
    void setYaw(float yawRadians) { m_yaw = yawRadians; }
    void setPitch(float pitchRadians) { m_pitch = pitchRadians; }
    void setWorldUp(const Vec3 &up) { m_worldUp = up; }

    void setFOV(float fovDegrees) { m_fov = fovDegrees; }
    void setAspectRatio(float aspect) { m_aspectRatio = aspect; }
    void setNearPlane(float nearPlane) { m_nearPlane = nearPlane; }
    void setFarPlane(float farPlane) { m_farPlane = farPlane; }

    const Vec3 &getTarget() const { return m_target; }
    const Vec3 &getPivotOffset() const { return m_pivotOffset; }
    float getDistance() const { return m_distance; }
    float getYaw() const { return m_yaw; }
    float getPitch() const { return m_pitch; }
    const Vec3 &getWorldUp() const { return m_worldUp; }

    float getFOV() const { return m_fov; }
    float getAspectRatio() const { return m_aspectRatio; }
    float getNearPlane() const { return m_nearPlane; }
    float getFarPlane() const { return m_farPlane; }

    Vec3 getEyePosition() const;

    Mat4 getViewMatrix() const;
    Mat4 getProjectionMatrix() const;
    /** `P * V` for `clip = P * V * M * pos`. */
    Mat4 getViewProjectionMatrix() const;

private:
    Vec3 m_target{};
    Vec3 m_pivotOffset{0.0f, 0.35f, 0.0f};
    float m_distance{6.0f};
    float m_yaw{0.0f};
    float m_pitch{-0.25f};
    Vec3 m_worldUp{0.0f, 1.0f, 0.0f};

    float m_fov{55.0f};
    float m_aspectRatio{16.0f / 9.0f};
    float m_nearPlane{0.1f};
    float m_farPlane{100.0f};
};

} // namespace Nebula
