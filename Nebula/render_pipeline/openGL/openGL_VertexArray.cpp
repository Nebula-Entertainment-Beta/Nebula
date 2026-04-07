#include "openGL_VertexArray.h"
#include "openGL_IndexBuffer.h"
#include <glad/glad.h>
#include <iostream>






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
        if (!vertexBuffer) {
            std::cerr << "OpenGL_VertexArray::addVertexBuffer: null VertexBuffer\n";
            return;
        }
        bind();
        vertexBuffer->bind();
        const auto& layout = vertexBuffer->getlayout();

        for(const auto& element : layout.elements){
            glEnableVertexAttribArray(element.location);

            switch(element.type){
                case VertexAttributeType::Float:
                glVertexAttribPointer(
                    element.location,
                    element.componentCount,
                    GL_FLOAT,
                    element.normalized ? GL_TRUE : GL_FALSE,
                    static_cast<GLsizei>(layout.strideBytes),
                    reinterpret_cast<const void*>(element.offsetBytes));

                break;

                case VertexAttributeType::Int:
                glVertexAttribIPointer(
                    element.location,
                    element.componentCount,
                    GL_INT,
                    static_cast<GLsizei>(layout.strideBytes),
                    reinterpret_cast<const void*>(element.offsetBytes));

                break;

                case VertexAttributeType::UInt:
                glVertexAttribIPointer(
                    element.location,
                    element.componentCount,
                    GL_UNSIGNED_INT,
                    static_cast<GLsizei>(layout.strideBytes),
                    reinterpret_cast<const void*>(element.offsetBytes));

                break;

                default:
                    std::cerr << "OpenGL_VertexArray: unknown VertexAttributeType\n";
                    break;

            }
        }
        m_vertexBuffers.push_back(vertexBuffer);
        vertexBuffer->unbind();
        unbind();
    }

    void OpenGL_VertexArray::setIndexBuffer(const std::shared_ptr<IndexBuffer>& indexBuffer) {
        if (!indexBuffer) {
            std::cerr << "OpenGL_VertexArray::setIndexBuffer: null IndexBuffer\n";
            return;
        }
        bind();
        indexBuffer->bind();
        m_indexBuffer = indexBuffer;
        unbind();
    }

    const std::vector<std::shared_ptr<VertexBuffer>>& OpenGL_VertexArray::getVertexBuffers() const {
        
        return m_vertexBuffers;
    }

    const std::shared_ptr<IndexBuffer>& OpenGL_VertexArray::getIndexBuffer() const {
        
        return m_indexBuffer;
    }

}

    