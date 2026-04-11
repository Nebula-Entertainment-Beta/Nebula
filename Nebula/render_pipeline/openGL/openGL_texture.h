#pragma once
#include "texture.h"
#include <memory>
#include <cstdint>
#include <string>


namespace Nebula{
  class OpenGL_Texture : public Texture{
    public:
        OpenGL_Texture(int width, int height, const void* rgba);
        ~OpenGL_Texture() override;

        void bind(uint32_t textureUnit) const override;
        void unbind(uint32_t textureUnit) const override;
        int getWidth() const override;
        int getHeight() const override;

        static std::shared_ptr<OpenGL_Texture> create(int width, int height, const void* rgba);

    private:
        uint32_t m_textureID=0;
        int m_width=0;
        int m_height=0;
  };
}