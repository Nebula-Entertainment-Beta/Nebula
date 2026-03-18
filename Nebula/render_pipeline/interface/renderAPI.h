#pragma once
#include "renderer.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>

namespace Nebula
{
    enum class RendererAPIType{
      None =0,
      OpenGL,
      Vulkan,
    };

    class RenderAPI 
    {
    public:
      virtual ~RenderAPI() = default;
      virtual void init() = 0;
      virtual void Shutdown() = 0;
      virtual void drawIndexed(const std::shared_ptr<VertexArray>& vertexArray, uint32_t indexCount = 0) = 0;
      virtual void setViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) = 0;
      virtual void clear(const glm::vec4& color) = 0;
      static RendererAPIType getAPI();
    private:
      static RendererAPIType s_API;
      
      
      

    };

    

    std::unique_ptr<RenderAPI> createrendererAPI();

}