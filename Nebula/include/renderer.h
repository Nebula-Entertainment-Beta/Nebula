/**
 * @file renderer.h
 * @brief Simple static façade over `RenderAPI` — what most game code should call.
 *
 * Call `Renderer::init()` once after you have a valid OpenGL context (`Window::isValid()`), then
 * each frame: `clear`, draw meshes with `drawIndexed`, and present with `Window::swapBuffers()` (event
 * polling is separate — `Application::run` uses `pollEvents` before `onUpdate`).
 */
#pragma once
#include <memory>

#include "math_types.h"

namespace Nebula
{
    class VertexArray;
    class RenderAPI;

    /**
     * @brief Static wrapper around the active graphics backend.
     */
    class Renderer
    {
    public:
        static void init();
        static void Shutdown();
        static void drawIndexed(const std::shared_ptr<VertexArray>& vertexArray,
                                uint32_t indexCount = 0);
        static void setViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height);
        static void clear(const Vec4& color);

    private:
        static std::unique_ptr<RenderAPI> s_renderAPI;
    };

}
