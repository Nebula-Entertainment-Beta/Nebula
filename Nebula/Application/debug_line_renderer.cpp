#include "debug_line_renderer.h"

#include <glad/glad.h>

namespace Nebula
{

  void DebugLineRenderer::ensureInitialized()
  {
    if (m_initialized)
    {
      return;
    }
    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);
    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, nullptr);
    glBindVertexArray(0);
    m_initialized = true;
  }

  void DebugLineRenderer::draw(const std::vector<Vec3> &lines, const Mat4 &mvp, Shader &shader,
                               const Vec3 &color)
  {
    if (lines.empty())
    {
      return;
    }

    ensureInitialized();
    std::vector<float> packed;
    packed.reserve(lines.size() * 3);
    for (const Vec3 &p : lines)
    {
      packed.push_back(p.x);
      packed.push_back(p.y);
      packed.push_back(p.z);
    }

    shader.bind();
    shader.setMat4("uMVP", mvp);
    shader.setVec3("uColor", color);
    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(packed.size() * sizeof(float)),
                 packed.data(), GL_DYNAMIC_DRAW);
    glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(lines.size()));
    glBindVertexArray(0);
    shader.unbind();
  }

}
