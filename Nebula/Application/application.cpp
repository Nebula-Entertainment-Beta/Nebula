/**
 * @file application.cpp
 * @brief `Application` constructor, `run()` loop (`Renderer::init` … `Shutdown`), and default `onRender` (viewport + clear).
 */
#include "application.h"
#include "renderer.h"
#include "detail/openGL_GraphicsContext.h"

#include "math_types.h"

namespace Nebula
{

  Application::Application(const ApplicationSpec &spec) : m_window(spec.title, spec.width, spec.height), m_assets()
  {
    m_width = spec.width;
    m_height = spec.height;
    m_title = spec.title;
    m_rendererAPI = spec.rendererAPI;

    m_input.attach(m_window);

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
    auto &ctx = m_window.getGraphicsContext();
    ctx.makeCurrent();
    m_renderer.init(ctx, m_rendererAPI);
    m_rendererInitialized = true;
    m_hasRun = true;

    float lastTime = m_clock.nowSeconds();

    while (!m_window.shouldWindowClose())
    {
      m_input.beginFrame();
      double newtime = m_clock.nowSeconds();
      float dt = static_cast<float>(newtime - lastTime);
      lastTime = newtime;
      m_window.pollEvents();
      m_actionMapping.updateMappings(m_input);
      onUpdate(dt);
      onRender();
      ctx.swap();
    }
    m_renderer.Shutdown();
    m_rendererInitialized = false;
  }

  Application::~Application()
  {

    if (m_rendererInitialized)
    {
      m_renderer.Shutdown();
    }
    m_input.detach();
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
      m_renderer.setViewport(0, 0, static_cast<uint32_t>(fbw), static_cast<uint32_t>(fbh));
    }
    else
    {
      m_renderer.setViewport(0, 0, m_width, m_height);
    }
    m_renderer.clear(Vec4{0.1f, 0.1f, 0.15f, 1.0f});
  }

}
