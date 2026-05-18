/**
 * @file renderResources.h
 * @brief Backend-specific factory for GPU resources (shaders, buffers, textures).
 */
#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>

namespace Nebula
{
  class IndexBuffer;
  class Shader;
  class Texture;
  class VertexArray;
  class VertexBuffer;
  enum class BufferUsage;
  struct VertexBufferLayout;

  class IRenderResourceFactory
  {
  public:
    virtual ~IRenderResourceFactory() = default;

    virtual std::shared_ptr<Shader> createShader(const std::string &vertexSrc,
                                                   const std::string &fragmentSrc) = 0;
    virtual std::shared_ptr<Texture> createTexture(int width, int height, const void *rgba) = 0;
    virtual std::shared_ptr<VertexArray> createVertexArray() = 0;
    virtual std::shared_ptr<VertexBuffer> createVertexBuffer(const void *data,
                                                             size_t sizeBytes,
                                                             BufferUsage usage,
                                                             const VertexBufferLayout &layout) = 0;
    virtual std::shared_ptr<IndexBuffer> createIndexBuffer(uint32_t *indices, uint32_t count) = 0;
  };
}
