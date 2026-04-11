/**
 * @file index_buffer.cpp
 * @brief Dispatches `IndexBuffer::create` to the OpenGL EBO implementation.
 */
#include "index_buffer.h"
#include "../openGL/openGL_IndexBuffer.h"
#include "renderAPI.h"

namespace Nebula{
  std::shared_ptr<IndexBuffer> IndexBuffer::create( uint32_t* indices, uint32_t count){
    switch (RenderAPI::getAPI()){
      case RendererAPIType::OpenGL:
        return std::make_shared<OpenGL_IndexBuffer>(indices, count);
      default:
        return nullptr;
    }
  }
}