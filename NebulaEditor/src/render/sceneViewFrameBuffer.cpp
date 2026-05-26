#include "sceneViewFrameBuffer.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>

namespace Editor
{

  void SceneViewFrameBuffer::resize(int w, int h)
  {
    if (w == m_width && h == m_height)
      return;

    m_width = w;
    m_height = h;

    if (m_fbo == 0)
    {
      glGenFramebuffers(1, &m_fbo);
      glGenTextures(1, &m_colorTex);
      glGenRenderbuffers(1, &m_depthRbo);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

    glBindTexture(GL_TEXTURE_2D, m_colorTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_width, m_height, 0, GL_RGB,
                 GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                           m_colorTex, 0);

    glBindRenderbuffer(GL_RENDERBUFFER, m_depthRbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, m_width,
                          m_height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,
                              GL_RENDERBUFFER, m_depthRbo);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
      // Handle framebuffer creation error
      // For simplicity, we won't do anything here in this example
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
  }

  void SceneViewFrameBuffer::bind()
  {
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
  }

  void SceneViewFrameBuffer::unbind()
  {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
  }
  void SceneViewFrameBuffer::clear()
  {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  }

}