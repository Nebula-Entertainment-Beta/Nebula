#pragma once
#include <memory>
#include <iostream>
#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <glm/glm.hpp>



namespace Nebula
{
    class VertexArray;
    class RenderAPI;
    
    class Renderer {

      public:
          static void init();
          static void Shutdown();
          static void BeginFrame();
          static void EndFrame();
          static void drawIndexed(const std::shared_ptr<VertexArray>& vertexArray, uint32_t indexCount = 0);
          static void setViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height);
          static void clear(const glm::vec4& color);

      private:
            static std::unique_ptr<RenderAPI> s_renderAPI;
            

    };


}