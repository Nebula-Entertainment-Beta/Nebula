#pragma once

#include <memory>

#include "renderAPI.h"
#include "renderResources.h"
#include "renderer_api_type.h"

namespace Nebula
{
  struct RenderBackend
  {
    std::unique_ptr<RenderAPI> api;
    std::unique_ptr<IRenderResourceFactory> resources;
  };

  RenderBackend createOpenGLRenderBackend();
  RenderBackend createVulkanRenderBackend();
  RenderBackend createRenderBackend(RendererAPIType api);
}
