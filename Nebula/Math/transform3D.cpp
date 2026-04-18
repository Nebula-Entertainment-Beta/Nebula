/**
 * @file transform3D.cpp
 * @brief Builds column-major model matrix: T * R_y(yaw) * S(uniform).
 */
#include "transform3D.h"

#include <glm/gtc/matrix_transform.hpp>

namespace Nebula {

glm::mat4 Transform3D::getModelMatrix() const
{
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, m_position);
    model = glm::rotate(model, m_yaw, glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::scale(model, glm::vec3(m_scale));
    return model;
}

} // namespace Nebula
