#include "mesh_upload.h"

#include "index_buffer.h"
#include "vertex_array.h"
#include "vertex_buffer.h"

#include <cstddef>
#include <iostream>
#include <vector>

namespace Nebula
{
  namespace
  {
    VertexBufferLayout layoutPositionOnly()
    {
      VertexBufferLayout layout{};
      layout.strideBytes = sizeof(MeshVertex);
      VertexBufferElement pos{};
      pos.location = 0;
      pos.componentCount = 3;
      pos.type = VertexAttributeType::Float;
      pos.offsetBytes = offsetof(MeshVertex, x);
      pos.normalized = false;
      layout.elements.push_back(pos);
      VertexBufferElement uv{};
      uv.location = 1;
      uv.componentCount = 2;
      uv.type = VertexAttributeType::Float;
      uv.offsetBytes = offsetof(MeshVertex, u);
      uv.normalized = false;
      layout.elements.push_back(uv);
      return layout;
    }
  }

  bool uploadMeshAsset(IRenderResourceFactory &resources, const MeshAsset &asset, Mesh &outMesh)
  {
    if (asset.vertices.empty() || asset.indices.empty())
    {
      return false;
    }

    const VertexBufferLayout layout = layoutPositionOnly();
    const uint32_t indexCount = static_cast<uint32_t>(asset.indices.size());
    outMesh.indexCount = indexCount;

    auto vbo = resources.createVertexBuffer(
        asset.vertices.data(),
        asset.vertices.size() * sizeof(MeshVertex),
        BufferUsage::Static,
        layout);
    if (!vbo)
    {
      std::cerr << "uploadMeshAsset: VertexBuffer creation failed.\n";
      return false;
    }

    auto vao = resources.createVertexArray();
    if (!vao)
    {
      std::cerr << "uploadMeshAsset: VertexArray creation failed.\n";
      return false;
    }
    vao->addVertexBuffer(vbo);

    std::vector<uint32_t> indices(asset.indices.begin(), asset.indices.end());
    auto ibo = resources.createIndexBuffer(indices.data(), indexCount);
    if (!ibo)
    {
      std::cerr << "uploadMeshAsset: IndexBuffer creation failed.\n";
      return false;
    }
    vao->setIndexBuffer(ibo);

    outMesh.vao = std::move(vao);
    return true;
  }
}
