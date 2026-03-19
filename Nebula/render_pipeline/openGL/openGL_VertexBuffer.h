#pragma once
#include <cstddef>
#include <cstdint>
#include <memory>

#include "vertex_buffer.h"

namespace Nebula{
  class OpenGL_VertexBuffer : public VertexBuffer{
    public:
      OpenGL_VertexBuffer(
        const void* data,
        size_t sizeBytes,
        BufferUsage usage,
        const vertexBufferLayout& layout
      );
      ~OpenGL_VertexBuffer() override;
      void bind() const override;
      void unbind() const override;

      const vertexBufferLayout& getlayout() const override {
        return m_layout;
      }

      static std::shared_ptr<VertexBuffer> create(
        const void* data,
        size_t sizeBytes,
        BufferUsage usage,
        const vertexBufferLayout& layout);
    
    private:
        uint32_t m_rendererID =0;
        vertexBufferLayout m_layout;
  };
}