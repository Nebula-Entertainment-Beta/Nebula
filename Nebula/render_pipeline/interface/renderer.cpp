#include "renderer.h"
#include "renderAPI.h"
#include "vertex_array.h"

namespace Nebula {

  std::unique_ptr<RenderAPI> Renderer::s_renderAPI =nullptr;

  void Renderer::init() {
      // Initialization code
      // For example, you might want to create an instance of a specific RenderAPI implementation
      if(s_renderAPI){
        s_renderAPI->init();
      }
     
  }

  void Renderer::Shutdown() {
    s_renderAPI.reset();

  }

  
  void Renderer::drawIndexed(const std::shared_ptr<VertexArray>& vertexArray, uint32_t indexCount) {
    if(s_renderAPI) {
      s_renderAPI->drawIndexed(vertexArray, indexCount);
    }
  }
  void Renderer::setViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) {
    if(s_renderAPI) {
      s_renderAPI->setViewport(x, y, width, height);
    }
  }
  void Renderer::clear(const glm::vec4& color) {
    if(s_renderAPI) {
      s_renderAPI->clear(color);
    }
  }

} // namespace Nebula