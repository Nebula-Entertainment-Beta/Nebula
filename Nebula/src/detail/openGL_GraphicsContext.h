#pragma once
#include "graphicsContext.h"
#include <GLFW/glfw3.h>

namespace Nebula::detail
{

  class OpenGLGraphicsContext final : public graphicsContext
  {
  public:
    explicit OpenGLGraphicsContext(GLFWwindow *window);
    void makeCurrent() override { glfwMakeContextCurrent(m_window); }
    void swap() override { glfwSwapBuffers(m_window); }
    bool isValid() const override { return m_window != nullptr; }
    void getFramebufferSize(int &w, int &h) const override;

  private:
    GLFWwindow *m_window = nullptr;
  };
}