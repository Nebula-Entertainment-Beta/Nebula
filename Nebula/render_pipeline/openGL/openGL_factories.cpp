#include "openGL_RenderBackends.h"
#include "openGL_VertexBuffer.h"
#include "openGL_VertexArray.h"
#include "openGL_IndexBuffer.h"
#include "openGL_Shader.h"
#include "openGL_texture.h"
#include "openGL_Renderer.h"
namespace Nebula
{
  std::shared_ptr<Shader> Shader::create(const std::string &vertexSrc, const std::string &fragmentSrc)
  {
    switch (RenderAPI::getAPI())
    {
    case RendererAPIType::OpenGL:
      return OpenGL_Shader::create(vertexSrc, fragmentSrc);
    default:
      return nullptr;
    }
  }

  std::shared_ptr<Texture> Texture::create(int width, int height, const void *rgba)
  {
    switch (RenderAPI::getAPI())
    {
    case RendererAPIType::OpenGL:
      return OpenGL_Texture::create(width, height, rgba);
    default:
      return nullptr;
    }
  }

    std::shared_ptr<VertexArray> VertexArray::create()
  {

    switch (RenderAPI::getAPI())
    {

    case RendererAPIType::OpenGL:
      return std::make_shared<OpenGL_VertexArray>();

    default:
      return nullptr;
    }
  }

  std::shared_ptr<VertexBuffer> VertexBuffer::create(
      const void *data,
      size_t sizeBytes,
      BufferUsage usage,
      const VertexBufferLayout &layout)
  {

    switch (RenderAPI::getAPI())
    {
    case RendererAPIType::OpenGL:
      return OpenGL_VertexBuffer::create(data, sizeBytes, usage, layout);
    default:
      return nullptr;
    }
  }

  std::shared_ptr<IndexBuffer> IndexBuffer::create(uint32_t *indices, uint32_t count)
  {
    switch (RenderAPI::getAPI())
    {
    case RendererAPIType::OpenGL:
      return std::make_shared<OpenGL_IndexBuffer>(indices, count);
    default:
      return nullptr;
    }
  }

  std::unique_ptr<RenderAPI> createOpenGLRendererAPI()
  {
    return std::make_unique<OpenGL_Renderer>();
  }

  // ... same for IndexBuffer, Shader, Texture
}