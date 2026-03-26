#include "openGL_IndexBuffer.h"
#include <glad/glad.h>

namespace Nebula {
    OpenGL_IndexBuffer::OpenGL_IndexBuffer(uint32_t* indices, uint32_t count) : m_count(count) {
        glGenBuffers(1, &m_rendererID);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_rendererID);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(uint32_t), indices, GL_STATIC_DRAW);
    }

    OpenGL_IndexBuffer::~OpenGL_IndexBuffer() {
        glDeleteBuffers(1, &m_rendererID);
    }

    void OpenGL_IndexBuffer::bind() const {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_rendererID);
    }

    void OpenGL_IndexBuffer::unbind() const {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }

    uint32_t OpenGL_IndexBuffer::getCount() const {
        return m_count;
    }
}