/**
 * @file openGL_Renderer.h
 * @brief OpenGL implementation of `RenderAPI` (`glDrawElements`, `glClear`, `glViewport`, …).
 */
#pragma once
#include <glad/glad.h>
#include "math_types.h"
#include "renderAPI.h"

namespace Nebula
{
    class OpenGL_Renderer : public RenderAPI
    {
    public:
        void init() override;
        void setViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) override;
        void clear(const Vec4& color) override;
        void drawIndexed(const std::shared_ptr<VertexArray>& vertexArray,
                         uint32_t indexCount = 0) override;
        void Shutdown() override;
    };
}
