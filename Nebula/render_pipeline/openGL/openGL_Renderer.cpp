/**
 * @file openGL_Renderer.cpp
 * @brief Issues indexed draws and frame setup for the OpenGL backend.
 */
#include "openGL_Renderer.h"
#include "../interface/vertex_array.h"
#include "../interface/index_buffer.h"

namespace Nebula
{
    void OpenGL_Renderer::init()
    {
        // Initialization code for OpenGL
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        

    }

    void OpenGL_Renderer::drawIndexed(const std::shared_ptr<VertexArray>& vertexArray, uint32_t indexCount)
    {
        if (!vertexArray)
            return;

        if (indexCount == 0) {
            const auto& ibo = vertexArray->getIndexBuffer();
            if (!ibo)
                return;
            indexCount = ibo->getCount();
        }
        if (indexCount == 0)
            return;

        vertexArray->bind();
        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indexCount), GL_UNSIGNED_INT, nullptr);
        vertexArray->unbind();
    }

    void OpenGL_Renderer::Shutdown()
    {
        // Cleanup code for OpenGL
        glDisable(GL_BLEND);
        glDisable(GL_DEPTH_TEST);

    }

    void OpenGL_Renderer::setViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
    {
        glViewport(x, y, width, height);
        
    }

    void OpenGL_Renderer::clear(const glm::vec4& color)
    {
        glClearColor(color.r, color.g, color.b, color.a);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    
  
}