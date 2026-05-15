/**
 * @file transform3D.h
 * @brief **3D transform** (translation, yaw, uniform scale) and a column-major model matrix.
 */
#pragma once

#include "math_types.h"

namespace Nebula
{

class Transform3D
{
public:
    Transform3D()
        : m_position{}
        , m_yaw(0.0f)
        , m_scale(1.0f)
    {
    }

    void setPosition(const Vec3 &pos) { m_position = pos; }
    void setYaw(float yawRadians) { m_yaw = yawRadians; }
    void setScale(float scale) { m_scale = scale; }

    const Vec3 &getPosition() const { return m_position; }
    float getYaw() const { return m_yaw; }
    float getScale() const { return m_scale; }

    Mat4 getModelMatrix() const;

private:
    Vec3 m_position;
    float m_yaw;
    float m_scale;
};

} // namespace Nebula
