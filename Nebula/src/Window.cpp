#include "Window.h"
#include <GLFW/glfw3.h>
#include <iostream>
#include <string_view>

namespace Nebula
{
  Window::Window(std::string_view title, int width, int height)
  {
      if(!glfwInit())
      {
          std::cerr << "Failed to initialize GLFW" << std::endl;
          return;
      }

      m_window = glfwCreateWindow(width, height, title.data(), nullptr, nullptr);
      if(m_window == NULL)
      {
          std::cerr << "Failed to create GLFW window" << std::endl;
      }
      glfwMakeContextCurrent(m_window);
  }
  
  void Window::update()
  {
      if (!m_window)
      {
          std::cerr << "Failed to update GLFW window" << std::endl;
          return;
      }
      glfwSwapBuffers(m_window);
      glfwPollEvents();
  }

  bool Window::shouldWindowClose()
  {
      return glfwWindowShouldClose(m_window);
  }

  Window::~Window()
  {
      glfwDestroyWindow(m_window);
  }
}
