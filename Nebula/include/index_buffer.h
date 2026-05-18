/**
 * @file index_buffer.h
 * @brief GPU buffer of **indices** (unsigned ints) that select which vertices form each triangle.
 *
 * Instead of duplicating corner vertices, you store unique vertices once and list triples of indices
 * (a “triangle list”). `getCount()` is how many indices exist — often a multiple of 3.
 */
#pragma once
#include <cstdint>
#include <memory>

namespace Nebula {

    class IndexBuffer
    {
    public:
        virtual ~IndexBuffer() = default;
        virtual void bind() const = 0;
        virtual void unbind() const = 0;
        virtual uint32_t getCount() const = 0;
    };
}
