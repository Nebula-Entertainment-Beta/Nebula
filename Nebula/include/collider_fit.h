#pragma once

#include "component.h"
#include "mesh_asset.h"
#include "physics/physics_component.h"

namespace Nebula
{

  class AssetManager;

  /** Local-space half extents from mesh vertex bounds (matches rendered mesh before entity scale). */
  Vec3 meshAssetLocalHalfExtents(const MeshAsset &mesh, float minHalfAxis = 0.05f);

  bool fitBoxColliderToMesh(ColliderComponent &collider, const MeshAsset &mesh);

  bool fitBoxColliderToMeshRenderer(ColliderComponent &collider, AssetManager &assets,
                                    const MeshRendererComponent &meshRenderer);

}
