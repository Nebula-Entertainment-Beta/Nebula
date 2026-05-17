#include "mesh.h"
#include "vertex_buffer.h"
#include "index_buffer.h"
#include "vertex_array.h"

#include <vector>
#include <iostream>

namespace Nebula
{
  namespace
  {
    struct Vertex3
    {
      float x, y, z;
    };

    VertexBufferLayout layoutPositionOnly()
    {
      VertexBufferLayout layout{};
      layout.strideBytes = sizeof(Vertex3);
      VertexBufferElement pos{};
      pos.location = 0;
      pos.componentCount = 3;
      pos.type = VertexAttributeType::Float;
      pos.offsetBytes = 0;
      pos.normalized = false;
      layout.elements.push_back(pos);
      return layout;
    }

    bool uploadIndexedMesh(
        const std::vector<Vertex3> &verts,
        const std::vector<uint32_t> &indices,
        Mesh &outMesh)
    {
      const VertexBufferLayout layout = layoutPositionOnly();
      const uint32_t indexCount = static_cast<uint32_t>(indices.size());
      outMesh.indexCount = indexCount;

      auto vbo = VertexBuffer::create(
          verts.data(),
          verts.size() * sizeof(Vertex3),
          BufferUsage::Static,
          layout);
      if (!vbo)
      {
        std::cerr << "VertexBuffer::create failed.\n";
        return false;
      }

      auto vao = VertexArray::create();
      if (!vao)
      {
        std::cerr << "VertexArray::create failed.\n";
        return false;
      }
      vao->addVertexBuffer(vbo);

      std::vector<uint32_t> indicesMutable(indices.begin(), indices.end());
      auto ibo = IndexBuffer::create(indicesMutable.data(), indexCount);
      if (!ibo)
      {
        std::cerr << "IndexBuffer::create failed.\n";
        return false;
      }
      vao->setIndexBuffer(ibo);

      outMesh.vao = std::move(vao);
      return true;
    }
  } // namespace

  bool buildBuiltinCubeMesh(Mesh &out)
  {
    const std::vector<Vertex3> verts = {
        {-0.5f, -0.5f, 0.5f},
        {0.5f, -0.5f, 0.5f},
        {0.5f, 0.5f, 0.5f},
        {-0.5f, 0.5f, 0.5f},
        {-0.5f, -0.5f, -0.5f},
        {0.5f, -0.5f, -0.5f},
        {0.5f, 0.5f, -0.5f},
        {-0.5f, 0.5f, -0.5f},
    };
    const std::vector<uint32_t> indices = {
        0, 1, 2, 0, 2, 3, 1, 5, 6, 1, 6, 2, 5, 4, 7, 5, 7, 6,
        4, 0, 3, 4, 3, 7, 3, 2, 6, 3, 6, 7, 4, 5, 1, 4, 1, 0};
    return uploadIndexedMesh(verts, indices, out);
  }

  bool buildBuiltinGroundMesh(Mesh &out)
  {
    const std::vector<Vertex3> verts = {
        {-12.0f, 0.0f, -12.0f},
        {12.0f, 0.0f, -12.0f},
        {12.0f, 0.0f, 12.0f},
        {-12.0f, 0.0f, 12.0f},
    };
    const std::vector<uint32_t> indices = {0, 1, 2, 0, 2, 3};
    return uploadIndexedMesh(verts, indices, out);
  }

} // namespace Nebula