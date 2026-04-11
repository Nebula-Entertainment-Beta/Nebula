#pragma once
#include <cstdint>
#include <memory>

/**
 * @file texture.h
 * @brief Abstract 2D GPU texture; concrete backends implement upload and bind.
 */

namespace Nebula{
  /**
   * @brief API-agnostic 2D texture (RGBA8 from memory in the OpenGL backend).
   */
  class Texture{
  public:
      virtual ~Texture() = default;

      /**
       * @brief Makes this texture active on texture unit `textureUnit` (0 = `GL_TEXTURE0`, 1 = `GL_TEXTURE1`, …).
       * @details Match this with `uniform sampler2D` in the shader (set the uniform to the same unit index).
       */
      virtual void bind(uint32_t textureUnit) const = 0;

      /** Unbinds the 2D target on the given unit (leaves other units untouched). */
      virtual void unbind(uint32_t textureUnit) const = 0;
      virtual int getWidth() const = 0;
      virtual int getHeight() const = 0;

      /**
       * @brief Factory for the active render API (e.g. OpenGL).
       * @param width  Pixel width; must be > 0.
       * @param height Pixel height; must be > 0.
       * @param rgba   RGBA8 data, `width * height * 4` bytes.
       * @return New texture, or nullptr if arguments are invalid or upload fails.
       */
      static std::shared_ptr<Texture> create(int width, int height, const void* rgba);

  };
}