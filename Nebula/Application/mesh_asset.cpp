#include "mesh_asset.h"

#include <algorithm>
#include <limits>

namespace Nebula
{
  void computeMeshBounds(MeshAsset &mesh)
  {
    if (mesh.vertices.empty())
    {
      mesh.boundsMin = {};
      mesh.boundsMax = {};
      return;
    }

    mesh.boundsMin = Vec3{
        std::numeric_limits<float>::max(),
        std::numeric_limits<float>::max(),
        std::numeric_limits<float>::max()};
    mesh.boundsMax = Vec3{
        std::numeric_limits<float>::lowest(),
        std::numeric_limits<float>::lowest(),
        std::numeric_limits<float>::lowest()};

    for (const MeshVertex &v : mesh.vertices)
    {
      mesh.boundsMin.x = std::min(mesh.boundsMin.x, v.x);
      mesh.boundsMin.y = std::min(mesh.boundsMin.y, v.y);
      mesh.boundsMin.z = std::min(mesh.boundsMin.z, v.z);
      mesh.boundsMax.x = std::max(mesh.boundsMax.x, v.x);
      mesh.boundsMax.y = std::max(mesh.boundsMax.y, v.y);
      mesh.boundsMax.z = std::max(mesh.boundsMax.z, v.z);
    }
  }
}
