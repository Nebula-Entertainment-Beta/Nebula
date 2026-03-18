#include "openGL_VertexArray.h"
#include <GL/glew.h>






namespace Nebula{
    OpenGL_VertexArray::OpenGL_VertexArray() {
        // Constructor implementation
          glGenVertexArrays(1, &m_rendererID);
          
        
    }

    OpenGL_VertexArray::~OpenGL_VertexArray() {
        // Destructor implementation
          glDeleteVertexArrays(1, &m_rendererID);

    }

    void OpenGL_VertexArray::bind() const {
        // Bind the vertex array
        glBindVertexArray(m_rendererID);

    }

    void OpenGL_VertexArray::unbind() const {
        // Unbind the vertex array
        glBindVertexArray(0);
    }

    void OpenGL_VertexArray::addVertexBuffer(const std::shared_ptr<VertexBuffer>& vertexBuffer) {
        // Add a vertex buffer
        m_vertexBuffers.push_back(vertexBuffer);
    }

    void OpenGL_VertexArray::setIndexBuffer(const std::shared_ptr<IndexBuffer>& indexBuffer) {
        // Set the index buffer
        m_indexBuffer = indexBuffer;
    }

    const std::vector<std::shared_ptr<VertexBuffer>>& OpenGL_VertexArray::getVertexBuffers() const {
        // Return the vertex buffers
        return m_vertexBuffers;
    }

    const std::shared_ptr<IndexBuffer>& OpenGL_VertexArray::getIndexBuffer() const {
        // Return the index buffer
        return m_indexBuffer;
    }

    std::shared_ptr<VertexArray> OpenGL_VertexArray::create() {
        return std::make_shared<OpenGL_VertexArray>();
    }
}