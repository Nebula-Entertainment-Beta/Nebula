#include "Window.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <string_view>


namespace Nebula
{ 
 //the implementation of the window class the first function is the constructor intializing a window
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
      if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
      {
          std::cerr << "Failed to initialize GLAD" << std::endl;
          return;
      }
  }
  
  //the update function is responsible for swapping the buffers and polling events to keep the window responsive
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

  //a check for if the window is closed or not
  bool Window::shouldWindowClose()
  {
      return glfwWindowShouldClose(m_window);
  }

  //the destructor to clean up the resources by destroying the window when it is no longer needed
  Window::~Window()
  {
      glfwDestroyWindow(m_window);
  }
}
