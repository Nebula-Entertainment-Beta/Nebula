#include "renderBackend.h"
#include "openGL_Renderer.h"
#include "openGL_resource_factory.h"

namespace Nebula
{
  std::unique_ptr<RenderAPI> createOpenGLRendererAPI()
  {
    return std::make_unique<OpenGL_Renderer>();
  }

  RenderBackend createOpenGLRenderBackend()
  {
    return RenderBackend{
        createOpenGLRendererAPI(),
        std::make_unique<OpenGLResourceFactory>()};
  }
}
