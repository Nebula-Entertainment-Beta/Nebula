#include "builtinMeshes.h"

#include "mesh_asset.h"

#include <vector>

namespace Nebula
{
  namespace
  {
    bool buildIndexedMeshAsset(
        const std::vector<MeshVertex> &verts,
        const std::vector<uint32_t> &indices,
        MeshAsset &out)
    {
      out.vertices = verts;
      out.indices = indices;
      computeMeshBounds(out);
      return !out.vertices.empty() && !out.indices.empty();
    }
  }

  bool buildBuiltinCubeMeshAsset(MeshAsset &out)
  {
    const std::vector<MeshVertex> verts = {
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
    return buildIndexedMeshAsset(verts, indices, out);
  }

  bool buildBuiltinGroundMeshAsset(MeshAsset &out)
  {
    const std::vector<MeshVertex> verts = {
        {-12.0f, 0.0f, -12.0f},
        {12.0f, 0.0f, -12.0f},
        {12.0f, 0.0f, 12.0f},
        {-12.0f, 0.0f, 12.0f},
    };
    const std::vector<uint32_t> indices = {0, 1, 2, 0, 2, 3};
    return buildIndexedMeshAsset(verts, indices, out);
  }

}
