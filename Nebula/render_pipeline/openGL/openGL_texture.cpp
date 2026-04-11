#pragma once
#include "openGL_texture.h"
#include <glad/glad.h>

namespace Nebula {
    OpenGL_Texture::OpenGL_Texture(int width, int height, const void* rgba)
        : m_width(width), m_height(height)
    {
        glGenTextures(1, &m_textureID);
        glBindTexture(GL_TEXTURE_2D, m_textureID);

        // Set texture parameters (you can adjust these as needed)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        // Upload the texture data to the GPU
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgba);
        glBindTexture(GL_TEXTURE_2D, 0); // Unbind the texture
    }

    OpenGL_Texture::~OpenGL_Texture()
    {
      if(m_textureID == 0)
        glDeleteTextures(1, &m_textureID);
    }

    void OpenGL_Texture::bind(uint32_t textureUnit) const
    {
        glActiveTexture(GL_TEXTURE0 + textureUnit);
        glBindTexture(GL_TEXTURE_2D, m_textureID);
    }

    void OpenGL_Texture::unbind(uint32_t textureUnit) const
    {
        glActiveTexture(GL_TEXTURE0 + textureUnit);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    int OpenGL_Texture::getWidth() const
    {
        return m_width;
    }

    int OpenGL_Texture::getHeight() const
    {
        return m_height;
    }

    std::shared_ptr<OpenGL_Texture> OpenGL_Texture::create(int width, int height, const void* rgba)
    {
        // if load unload texture failed return nullptr
        if (!rgba) {
            return nullptr;
        }
        return std::make_shared<OpenGL_Texture>(width, height, rgba);
    }

}
