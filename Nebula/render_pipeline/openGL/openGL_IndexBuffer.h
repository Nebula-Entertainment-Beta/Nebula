/**
 * @file openGL_IndexBuffer.h
 * @brief OpenGL **element buffer** (`GL_ELEMENT_ARRAY_BUFFER`) for indexed drawing.
 *
 * @note On core profile, data is first uploaded via `GL_ARRAY_BUFFER` then bound as EBO on a VAO;
 *       see the `.cpp` comment for why.
 */
#pragma once
#include <cstdint>
#include <memory>
#include "../interface/index_buffer.h"

namespace Nebula {
    class OpenGL_IndexBuffer : public IndexBuffer {
    public:
        OpenGL_IndexBuffer(uint32_t* indices, uint32_t count);
        virtual ~OpenGL_IndexBuffer();

        void bind() const override;
        void unbind() const override;
        uint32_t getCount() const override;

    private:
        uint32_t m_rendererID;
        uint32_t m_count;
    };
}