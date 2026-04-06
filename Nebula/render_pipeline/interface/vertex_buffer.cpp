#include "vertex_buffer.h"
#include "../openGL/openGL_VertexBuffer.h"
#include "renderAPI.h"

namespace Nebula{

  std::shared_ptr<VertexBuffer> VertexBuffer::create(
        const void* data,
        size_t sizeBytes,
        BufferUsage usage,
        const VertexBufferLayout& layout){


        switch (RenderAPI::getAPI()){
          case RendererAPIType::OpenGL:
            return OpenGL_VertexBuffer::create(data,sizeBytes,usage,layout);
          default:
            return nullptr;
        }
      }

}