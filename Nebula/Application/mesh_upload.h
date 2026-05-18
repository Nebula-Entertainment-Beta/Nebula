#pragma once

#include "mesh.h"
#include "mesh_asset.h"
#include "renderResources.h"

namespace Nebula
{
  bool uploadMeshAsset(IRenderResourceFactory &resources, const MeshAsset &asset, Mesh &outMesh);
}
