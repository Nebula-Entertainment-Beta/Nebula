/**
 * @file vertex_array.h
 * @brief Binds together **vertex buffers** + **attribute layout** + optional **index buffer** for one draw call.
 *
 * **OpenGL mapping:** This wraps a **VAO** (Vertex Array Object). The VAO remembers which VBO
 * feeds which shader attribute and which EBO supplies triangle indices, so you `bind()` the VAO and
 * draw instead of rebinding everything every frame.
 */
#pragma once
#include <memory>
#include <vector>

namespace Nebula {

    class VertexBuffer;
    class IndexBuffer;

    /**
     * @brief Drawable mesh state: vertex format + optional indexed triangles.
     */
    class VertexArray
    {
    public:
        virtual ~VertexArray() = default;

        virtual void bind() const = 0;
        virtual void unbind() const = 0;

        /** Attaches a VBO and registers its layout with the underlying VAO. */
        virtual void addVertexBuffer(const std::shared_ptr<VertexBuffer>& vertexBuffer) = 0;

        /** Sets the index buffer used by `glDrawElements` / `drawIndexed`. */
        virtual void setIndexBuffer(const std::shared_ptr<IndexBuffer>& indexBuffer) = 0;

        virtual const std::vector<std::shared_ptr<VertexBuffer>>& getVertexBuffers() const = 0;
        virtual const std::shared_ptr<IndexBuffer>& getIndexBuffer() const = 0;

        static std::shared_ptr<VertexArray> create();
    };

}