#pragma once
#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>
#include <glm/glm.hpp>

namespace Nebula{
  enum class BufferUsage {Static, Dynamic, Stream};
  enum class VertexAttributeType{Float, Int, UInt};

  struct vertexBufferElement{
    uint32_t location =0;
    uint32_t componentCount =0;
    VertexAttributeType type = VertexAttributeType::Float;
    size_t offsetBytes = 0;
    bool normalized = false;

  };

  struct vertexBufferLayout {
    size_t strideBytes;
    std:: vector<vertexBufferElement> elements;
  };

  class VertexBuffer{

    public:
      virtual ~VertexBuffer() = default;
       virtual void bind() const = 0;
      virtual void unbind() const = 0;
      virtual const vertexBufferLayout& getlayout() const =0;



      static std::shared_ptr<VertexBuffer> create(
        const void* data,
        size_t sizeBytes,
        BufferUsage usage,
        const vertexBufferLayout& layout
      );

  };
}