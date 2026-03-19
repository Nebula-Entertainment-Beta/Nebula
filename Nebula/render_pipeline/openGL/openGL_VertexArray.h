#pragma once
#include <cstdint>
#include <memory>
#include <vector>
#include "vertex_array.h"
#include "../../render_pipeline/interface/vertex_buffer.h"


namespace Nebula {

    class OpenGL_VertexArray : public VertexArray {
    public:
        OpenGL_VertexArray();
       ~OpenGL_VertexArray();

        void bind() const override;
        void unbind() const override;

        void addVertexBuffer(const std::shared_ptr<VertexBuffer>& vertexBuffer) override;
        void setIndexBuffer(const std::shared_ptr<IndexBuffer>& indexBuffer) override;

        const std::vector<std::shared_ptr<VertexBuffer>>& getVertexBuffers() const override;
        const std::shared_ptr<IndexBuffer>& getIndexBuffer() const override;
        static std::shared_ptr<VertexArray> create();
    private:
        uint32_t m_rendererID =0;
        std::vector<std::shared_ptr<VertexBuffer>> m_vertexBuffers;
        std::shared_ptr<IndexBuffer> m_indexBuffer;
    };

} // namespace Nebula

