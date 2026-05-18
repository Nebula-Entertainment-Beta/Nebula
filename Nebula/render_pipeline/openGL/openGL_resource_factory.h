#pragma once

#include "renderResources.h"

namespace Nebula
{
  class OpenGLResourceFactory : public IRenderResourceFactory
  {
  public:
    std::shared_ptr<Shader> createShader(const std::string &vertexSrc,
                                           const std::string &fragmentSrc) override;
    std::shared_ptr<Texture> createTexture(int width, int height, const void *rgba) override;
    std::shared_ptr<VertexArray> createVertexArray() override;
    std::shared_ptr<VertexBuffer> createVertexBuffer(const void *data,
                                                     size_t sizeBytes,
                                                     BufferUsage usage,
                                                     const VertexBufferLayout &layout) override;
    std::shared_ptr<IndexBuffer> createIndexBuffer(uint32_t *indices, uint32_t count) override;
  };
}
