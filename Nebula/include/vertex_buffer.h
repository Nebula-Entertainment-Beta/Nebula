/**
 * @file vertex_buffer.h
 * @brief Describes **vertex data** on the GPU: bytes + how attributes (position, color, UV…) are laid out.
 *
 * **Mental model:** A mesh is a blob of floats/ints in a buffer. The **layout** tells OpenGL
 * “attribute 0 is 3 floats starting at byte 0, attribute 1 is 4 floats starting at byte 12…” so the
 * vertex shader receives `vec3 a_Position`, `vec4 a_Color`, etc.
 */
#pragma once
#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>

namespace Nebula {

    /** Hint for the driver: how often buffer contents change (affects performance tuning). */
    enum class BufferUsage { Static, Dynamic, Stream };

    /** Component type for `glVertexAttribPointer` / `glVertexAttribIPointer`. */
    enum class VertexAttributeType { Float, Int, UInt };

    /**
     * @brief One vertex attribute slot (shader `layout(location = N)` matches `location`).
     */
    struct VertexBufferElement {
        uint32_t location = 0;
        uint32_t componentCount = 0;
        VertexAttributeType type = VertexAttributeType::Float;
        size_t offsetBytes = 0;
        bool normalized = false;
    };

    /**
     * @brief Stride + list of elements for one interleaved vertex format.
     */
    struct VertexBufferLayout {
        size_t strideBytes;
        std::vector<VertexBufferElement> elements;
    };

    /** API-agnostic vertex buffer; create via `IRenderResourceFactory`. */
    class VertexBuffer
    {
    public:
        virtual ~VertexBuffer() = default;
        virtual void bind() const = 0;
        virtual void unbind() const = 0;
        virtual const VertexBufferLayout& getlayout() const = 0;
    };
}