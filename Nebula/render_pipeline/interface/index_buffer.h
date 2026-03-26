#pragma once
#include <cstdint>
#include <memory>
#include "vertex_buffer.h"

namespace Nebula{
  class IndexBuffer{
  public:
      virtual ~IndexBuffer() = default;
      virtual void bind() const = 0;
      virtual void unbind() const = 0;
      virtual uint32_t getCount() const = 0;

      static std::shared_ptr<IndexBuffer> create( uint32_t* indices, uint32_t count);
  };
}

