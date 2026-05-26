#pragma once

namespace Editor
{

  class SceneViewFrameBuffer
  {
  public:
    void resize(int w, int h);
    void bind();
    void unbind();
    void clear();
    unsigned int colorTextureId() const { return m_colorTex; }

  private:
    unsigned int m_fbo = 0, m_colorTex = 0, m_depthRbo = 0;
    int m_width = 0, m_height = 0;
  };

}