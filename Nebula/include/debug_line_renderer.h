#pragma once

#include "math_types.h"
#include "shader.h"

#include <vector>

namespace Nebula
{

  /** Shared GL line batch used by transform gizmos and collider debug draw. */
  class DebugLineRenderer
  {
  public:
    void draw(const std::vector<Vec3> &lines, const Mat4 &mvp, Shader &shader, const Vec3 &color);

  private:
    void ensureInitialized();

    unsigned int m_vao = 0;
    unsigned int m_vbo = 0;
    bool m_initialized = false;
  };

}
