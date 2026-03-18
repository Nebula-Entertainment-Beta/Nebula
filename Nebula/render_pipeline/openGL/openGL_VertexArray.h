
#include "vertex_array.h"

namespace Nebula {

    class OpenGL_VertexArray : public VertexArray {
    public:
        OpenGL_VertexArray();
        virtual ~OpenGL_VertexArray();

        virtual void bind() const override;
        virtual void unbind() const override;

        virtual void addVertexBuffer(const std::shared_ptr<VertexBuffer>& vertexBuffer) override;
        virtual void setIndexBuffer(const std::shared_ptr<IndexBuffer>& indexBuffer) override;

        virtual const std::vector<std::shared_ptr<VertexBuffer>>& getVertexBuffers() const override;
        virtual const std::shared_ptr<IndexBuffer>& getIndexBuffer() const override;
        static std::shared_ptr<VertexArray> create();
    private:
        std::vector<std::shared_ptr<VertexBuffer>> m_vertexBuffers;
        std::shared_ptr<IndexBuffer> m_indexBuffer;
    };

} // namespace Nebula

