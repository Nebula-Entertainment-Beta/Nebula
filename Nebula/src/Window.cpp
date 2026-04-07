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
      if (!glfwInit()) {
          std::cerr << "Failed to initialize GLFW" << std::endl;
          return;
      }
      m_glfwInitialized = true;

      // Request a context that matches generated GLAD (gl=4.1). Required for core + macOS.
      glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
      glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
      glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
      glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

      m_window = glfwCreateWindow(width, height, title.data(), nullptr, nullptr);
      if (!m_window) {
          std::cerr << "Failed to create GLFW window" << std::endl;
          glfwTerminate();
          m_glfwInitialized = false;
          return;
      }

      glfwMakeContextCurrent(m_window);

      if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
          std::cerr << "Failed to initialize GLAD (OpenGL loader). "
                       "GPU/driver may not support the requested context.\n";
          glfwDestroyWindow(m_window);
          m_window = nullptr;
          glfwTerminate();
          m_glfwInitialized = false;
          return;
      }

      m_glReady = true;
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
      if (!m_window)
          return true;
      return glfwWindowShouldClose(m_window) != 0;
  }

  //the destructor to clean up the resources by destroying the window when it is no longer needed
  Window::~Window()
  {
      if (m_window) {
          glfwDestroyWindow(m_window);
          m_window = nullptr;
      }
      if (m_glfwInitialized) {
          glfwTerminate();
          m_glfwInitialized = false;
      }
  }
}
