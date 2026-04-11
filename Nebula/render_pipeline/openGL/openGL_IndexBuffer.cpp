/**
 * @file openGL_IndexBuffer.cpp
 * @brief Uploads index data and binds it for `glDrawElements`.
 */
#include "openGL_IndexBuffer.h"
#include <glad/glad.h>

namespace Nebula {
    OpenGL_IndexBuffer::OpenGL_IndexBuffer(uint32_t* indices, uint32_t count) : m_count(count) {
        // Core profile: binding GL_ELEMENT_ARRAY_BUFFER requires a bound VAO. Upload index data
        // using GL_ARRAY_BUFFER first; the buffer object is the same when later bound as EBO.
        glGenBuffers(1, &m_rendererID);
        glBindBuffer(GL_ARRAY_BUFFER, m_rendererID);
        glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(count * sizeof(uint32_t)), indices,
                     GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
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