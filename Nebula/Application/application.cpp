/**
 * @file application.cpp
 * @brief Placeholder — application bootstrap will be implemented here later.
 */
#include "application.h"
#include "renderer.h"
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

namespace Nebula
{

  Application::Application(const ApplicationSpec &spec) : m_window(spec.title, spec.width, spec.height)
  {
    m_width = spec.width;
    m_height = spec.height;
    m_title = spec.title;

    m_input.attachToWindow(m_window.getGLFWwindow());

    // create default scene data
    m_scene = Scene();
  }

  void Application::run()
  {
    if (m_hasRun)
    {
      return;
    }
    if (!m_window.isValid())
    {
      return;
    }

    Renderer::init();
    m_rendererInitialized = true;
    m_hasRun = true;

    float lastTime = glfwGetTime();

    while (!m_window.shouldWindowClose())
    {
      m_input.beginFrame();
      double newtime = glfwGetTime();
      float dt = static_cast<float>(newtime - lastTime);
      lastTime = newtime;
      m_window.pollEvents();
      m_actionMapping.updateMappings(m_input);
      onUpdate(dt);
      onRender();
      m_window.swapBuffers();
    }
    Renderer::Shutdown();
    m_rendererInitialized = false;
  }

  Application::~Application()
  {

    if (m_rendererInitialized)
    {
      Renderer::Shutdown();
    }
    m_input.detachFromWindow();
    m_rendererInitialized = false;
  }

  void Application::onUpdate(float dt)
  {
  }
  void Application::onRender()
  {
    int fbw = 0;
    int fbh = 0;
    m_window.getFramebufferSize(fbw, fbh);
    if (fbw > 0 && fbh > 0)
    {
      Renderer::setViewport(0, 0, static_cast<uint32_t>(fbw), static_cast<uint32_t>(fbh));
    }
    else
    {
      Renderer::setViewport(0, 0, m_width, m_height);
    }
    Renderer::clear(glm::vec4(0.1f, 0.1f, 0.15f, 1.0f));
  }

}
