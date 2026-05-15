/**
 * @file transform3D.cpp
 * @brief Builds column-major model matrix: T * R_y(yaw) * S(uniform).
 */
#include "transform3D.h"

#include <glm/gtc/matrix_transform.hpp>

namespace Nebula {
namespace {

glm::vec3 toGlm(const Vec3 &v)
{
    return glm::vec3(v.x, v.y, v.z);
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

Mat4 Transform3D::getModelMatrix() const
{
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, toGlm(m_position));
    model = glm::rotate(model, m_yaw, glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::scale(model, glm::vec3(m_scale));
    return fromGlm(model);
}

} // namespace Nebula
