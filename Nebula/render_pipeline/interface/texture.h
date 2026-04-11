#pragma once
#include <cstdint>
#include <memory>

namespace Nebula{
  class Texture{
  public:
      virtual ~Texture() = default;
      virtual void bind(uint32_t textureUnit) const = 0;
      virtual void unbind(uint32_t textureUnit) const = 0;
      virtual int getWidth() const = 0;
      virtual int getHeight() const = 0;

      static std::shared_ptr<Texture> create(int width, int height, const void* rgba);

  };
}