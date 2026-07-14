#pragma once

#include <string>

#include "mesh_asset.h"

namespace Nebula
{

  struct MeshImportOptions
  {
    std::string sourcePath;
    int meshIndex = -1;
    std::string meshName;
    float importScale = 1.0f;
    /** "none", "center", or "center_bottom" */
    std::string pivot = "none";
  };

  bool parseMeshDescriptorJson(const std::string &jsonText, MeshImportOptions &out);

  void applyMeshImportPostProcess(MeshAsset &mesh, const MeshImportOptions &options);

} // namespace Nebula
