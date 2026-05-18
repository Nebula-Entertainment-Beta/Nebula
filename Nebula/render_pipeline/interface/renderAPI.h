/**
 * @file renderAPI.h
 * @brief Low-level graphics backend interface (OpenGL today; room for Vulkan later).
 *
 * **Beginner angle:** `Renderer` is the friendly front door; `RenderAPI` is the interface
 * each backend implements. Drawing always goes: prepare geometry (`VertexArray`), then
 * `drawIndexed`, which ends up calling OpenGL’s `glDrawElements` under the hood.
 */
#pragma once
#include <memory>

#include "math_types.h"

namespace Nebula
{
    class VertexArray;

    /**
     * @brief Per-backend operations: init state, clear the framebuffer, issue indexed draws.
     */
    class RenderAPI
    {
    public:
        virtual ~RenderAPI() = default;

        /** One-time GL state (depth test, blending, etc.). */
        virtual void init() = 0;
        virtual void Shutdown() = 0;

        /**
         * @brief Draw triangles using the bound vertex array and index buffer.
         * @param vertexArray Mesh + layout + optional index buffer.
         * @param indexCount  Number of indices to draw; `0` means “use full index buffer”.
         */
        virtual void drawIndexed(const std::shared_ptr<VertexArray> &vertexArray,
                                 uint32_t indexCount = 0) = 0;

        /** Maps normalized device coordinates to pixel region (usually full window). */
        virtual void setViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) = 0;

        /** Fills the color (and depth) buffers before a new frame. */
        virtual void clear(const Vec4 &color) = 0;
    };
}
