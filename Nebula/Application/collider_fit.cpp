#include "collider_fit.h"

#include "assetManager.h"

#include <algorithm>

namespace Nebula
{

  Vec3 meshAssetLocalHalfExtents(const MeshAsset &mesh, float minHalfAxis)
  {
    const Vec3 half{
        (mesh.boundsMax.x - mesh.boundsMin.x) * 0.5f,
        (mesh.boundsMax.y - mesh.boundsMin.y) * 0.5f,
        (mesh.boundsMax.z - mesh.boundsMin.z) * 0.5f,
    };
    return {
        std::max(half.x, minHalfAxis),
        std::max(half.y, minHalfAxis),
        std::max(half.z, minHalfAxis),
    };
  }

  bool fitBoxColliderToMesh(ColliderComponent &collider, const MeshAsset &mesh)
  {
    if (mesh.vertices.empty())
    {
      return false;
    }

    collider.shape = ColliderComponent::Shape::Box;
    collider.halfExtents = meshAssetLocalHalfExtents(mesh);
    return true;
  }

  bool fitBoxColliderToMeshRenderer(ColliderComponent &collider, const AssetManager &assets,
                                    const MeshRendererComponent &meshRenderer)
  {
    const MeshAsset *meshAsset = assets.getCpuMeshAsset(meshRenderer.m_meshPath);
    if (meshAsset == nullptr)
    {
      return false;
    }
    return fitBoxColliderToMesh(collider, *meshAsset);
  }

}
