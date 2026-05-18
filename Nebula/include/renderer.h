/**
 * @file renderer.h
 * @brief
 *
 * Call `Renderer::init()` once after you have a valid OpenGL context (`Window::isValid()`), then
 * each frame: `clear`, draw meshes with `drawIndexed`, and present with `Window::swapBuffers()` (event
 * polling is separate — `Application::run` uses `pollEvents` before `onUpdate`).
 */
#pragma once
#include <memory>
#include "graphicsContext.h"
#include "math_types.h"
#include "renderResources.h"
#include "renderer_api_type.h"

namespace Nebula
{
    class VertexArray;

    /**
     * @brief Owns the active graphics backend and its GPU resource factory.
     */
    class Renderer
    {
    public:
        Renderer();
        ~Renderer();

        Renderer(const Renderer &) = delete;
        Renderer &operator=(const Renderer &) = delete;
        Renderer(Renderer &&) = delete;
        Renderer &operator=(Renderer &&) = delete;

        void Shutdown();
        void drawIndexed(const std::shared_ptr<VertexArray> &vertexArray, uint32_t indexCount = 0);
        void setViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height);
        void clear(const Vec4 &color);
        void init(graphicsContext &ctx, RendererAPIType api);

        IRenderResourceFactory &resources();
        const IRenderResourceFactory &resources() const;

    private:
        struct Impl;
        std::unique_ptr<Impl> m_impl;
    };

}
