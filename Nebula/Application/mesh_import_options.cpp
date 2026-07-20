#include "mesh_import_options.h"

#include <filesystem>
#include <nlohmann/json.hpp>

#include <algorithm>
#include <iostream>

namespace Nebula
{

  bool parseMeshDescriptorJson(const std::string &jsonText, MeshImportOptions &out)
  {
    try
    {
      const auto json = nlohmann::json::parse(jsonText);
      if (!json.contains("source") || !json["source"].is_string())
      {
        return false;
      }
      out.sourcePath = json["source"].get<std::string>();
      if (json.contains("meshIndex") && json["meshIndex"].is_number_integer())
      {
        out.meshIndex = json["meshIndex"].get<int>();
      }
      if (json.contains("meshName") && json["meshName"].is_string())
      {
        out.meshName = json["meshName"].get<std::string>();
      }
      if (json.contains("importScale") && json["importScale"].is_number())
      {
        out.importScale = json["importScale"].get<float>();
      }
      if (json.contains("pivot") && json["pivot"].is_string())
      {
        out.pivot = json["pivot"].get<std::string>();
      }
      return true;
    }
    catch (const std::exception &ex)
    {
      std::cerr << "parseMeshDescriptorJson: " << ex.what() << '\n';
      return false;
    }
  }

  void applyMeshImportPostProcess(MeshAsset &mesh, const MeshImportOptions &options)
  {
    if (mesh.vertices.empty())
    {
      return;
    }

    if (options.importScale != 1.0f)
    {
      for (MeshVertex &v : mesh.vertices)
      {
        v.x *= options.importScale;
        v.y *= options.importScale;
        v.z *= options.importScale;
      }
      computeMeshBounds(mesh);
    }

    if (options.pivot == "none")
    {
      return;
    }

    computeMeshBounds(mesh);
    Vec3 offset{};
    if (options.pivot == "center")
    {
      offset = {
          (mesh.boundsMin.x + mesh.boundsMax.x) * 0.5f,
          (mesh.boundsMin.y + mesh.boundsMax.y) * 0.5f,
          (mesh.boundsMin.z + mesh.boundsMax.z) * 0.5f};
    }
    else if (options.pivot == "center_bottom")
    {
      offset = {
          (mesh.boundsMin.x + mesh.boundsMax.x) * 0.5f,
          mesh.boundsMin.y,
          (mesh.boundsMin.z + mesh.boundsMax.z) * 0.5f};
    }

    for (MeshVertex &v : mesh.vertices)
    {
      v.x -= offset.x;
      v.y -= offset.y;
      v.z -= offset.z;
    }
    computeMeshBounds(mesh);
  }

} // namespace Nebula
