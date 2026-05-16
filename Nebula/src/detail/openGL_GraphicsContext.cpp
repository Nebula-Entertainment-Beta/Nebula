#include "openGL_GraphicsContext.h"

namespace Nebula::detail
{

  OpenGLGraphicsContext::OpenGLGraphicsContext(GLFWwindow *window) : m_window(window)
  {
  }
  void OpenGLGraphicsContext::getFramebufferSize(int &w, int &h) const
  {
    if (!m_window)
    {
      w = 0;
      h = 0;
      return;
    }
    {
      glfwGetFramebufferSize(m_window, &w, &h);
    }
  }

}