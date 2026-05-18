#include "openGL_resource_factory.h"

#include "openGL_IndexBuffer.h"
#include "openGL_Shader.h"
#include "openGL_texture.h"
#include "openGL_VertexArray.h"
#include "openGL_VertexBuffer.h"

namespace Nebula
{
  std::shared_ptr<Shader> OpenGLResourceFactory::createShader(const std::string &vertexSrc,
                                                              const std::string &fragmentSrc)
  {
    return OpenGL_Shader::create(vertexSrc, fragmentSrc);
  }

  std::shared_ptr<Texture> OpenGLResourceFactory::createTexture(int width, int height, const void *rgba)
  {
    return OpenGL_Texture::create(width, height, rgba);
  }

  std::shared_ptr<VertexArray> OpenGLResourceFactory::createVertexArray()
  {
    return std::make_shared<OpenGL_VertexArray>();
  }

  std::shared_ptr<VertexBuffer> OpenGLResourceFactory::createVertexBuffer(
      const void *data,
      size_t sizeBytes,
      BufferUsage usage,
      const VertexBufferLayout &layout)
  {
    return OpenGL_VertexBuffer::create(data, sizeBytes, usage, layout);
  }

  std::shared_ptr<IndexBuffer> OpenGLResourceFactory::createIndexBuffer(uint32_t *indices, uint32_t count)
  {
    return std::make_shared<OpenGL_IndexBuffer>(indices, count);
  }
}
