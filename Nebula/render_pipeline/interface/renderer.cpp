/**
 * @file renderer.cpp
 * @brief Forwards calls to the concrete `RenderAPI` instance (e.g. `OpenGL_Renderer`).
 */
#include "renderer.h"
#include "renderBackend.h"
#include "vertex_array.h"

namespace Nebula
{
  struct Renderer::Impl
  {
    std::unique_ptr<RenderAPI> api;
    std::unique_ptr<IRenderResourceFactory> resources;
    graphicsContext *ctx = nullptr;
  };

  // Must be defined here (after Impl is complete) so TUs that include renderer.h
  // never instantiate unique_ptr<Impl>'s destructor — fixes incomplete-type errors.
  Renderer::Renderer() = default;
  Renderer::~Renderer() = default;

  void Renderer::Shutdown()
  {
    if (!m_impl || !m_impl->api)
    {
      return;
    }
    m_impl->api->Shutdown();
    m_impl->api.reset();
    m_impl->resources.reset();
  }

  void Renderer::drawIndexed(const std::shared_ptr<VertexArray> &vertexArray, uint32_t indexCount)
  {
    if (m_impl && m_impl->api)
    {
      m_impl->api->drawIndexed(vertexArray, indexCount);
    }
  }
  void Renderer::setViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
  {
    if (m_impl && m_impl->api)
    {
      m_impl->api->setViewport(x, y, width, height);
    }
  }
  void Renderer::clear(const Vec4 &color)
  {
    if (m_impl && m_impl->api)
    {
      m_impl->api->clear(color);
    }
  }

  IRenderResourceFactory &Renderer::resources()
  {
    return *m_impl->resources;
  }

  const IRenderResourceFactory &Renderer::resources() const
  {
    return *m_impl->resources;
  }

  void Renderer::init(graphicsContext &ctx, RendererAPIType api)
  {
    m_impl = std::make_unique<Impl>();
    if (!ctx.isValid())
    {
      return;
    }
    ctx.makeCurrent();
    m_impl->ctx = &ctx;

    RenderBackend backend = createRenderBackend(api);
    m_impl->api = std::move(backend.api);
    m_impl->resources = std::move(backend.resources);

    if (m_impl->api)
    {
      m_impl->api->init();
    }
  }

}
