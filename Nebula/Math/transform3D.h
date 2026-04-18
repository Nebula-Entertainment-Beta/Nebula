/**
 * @file transform3D.h
 * @brief Reserved for **3D transforms** (translation, rotation, scale) using glm or similar.
 *
 * Same idea as 2D, but with quaternions or Euler angles and a 4×4 model matrix for depth and
 * perspective-correct rendering.
 *
 */
#pragma once

#include <glm/glm.hpp>

namespace Nebula {

class Transform3D {
public:
    Transform3D()
        : m_position(0.0f)
        , m_yaw(0.0f)
        , m_scale(1.0f)
    {
    }

    void setPosition(const glm::vec3& pos) { m_position = pos; }
    void setYaw(float yawRadians) { m_yaw = yawRadians; }
    void setScale(float scale) { m_scale = scale; }

    const glm::vec3& getPosition() const { return m_position; }
    float getYaw() const { return m_yaw; }
    float getScale() const { return m_scale; }

    glm::mat4 getModelMatrix() const;

private:
    glm::vec3 m_position;
    float m_yaw;
    float m_scale;
};

} // namespace Nebula
