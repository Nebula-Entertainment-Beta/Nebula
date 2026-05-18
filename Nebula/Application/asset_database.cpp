#include "asset_database.h"

#include "mesh_importer.h"

#include <nlohmann/json.hpp>

#include <iostream>

namespace Nebula
{
  AssetDatabase::AssetDatabase(const IAssetProvider &assets) : m_assets(assets) {}

  AssetGuid AssetDatabase::loadOrGetMesh(std::string_view logicalPath)
  {
    if (const AssetGuid existing = guidForPath(logicalPath); existing != kInvalidAsset)
    {
      return existing;
    }

    MeshAsset imported{};
    if (!importMeshAsset(m_assets, logicalPath, imported))
    {
      return kInvalidAsset;
    }

    registerMeshAsset(logicalPath, std::move(imported));
    return guidForPath(logicalPath);
  }

  AssetGuid AssetDatabase::loadOrGetMaterial(std::string_view logicalPath)
  {
    if (const AssetGuid existing = guidForPath(logicalPath); existing != kInvalidAsset)
    {
      return existing;
    }

    std::vector<uint8_t> bytes;
    if (!m_assets.readFile(logicalPath, bytes))
    {
      std::cerr << "AssetDatabase: failed to read material " << logicalPath << '\n';
      return kInvalidAsset;
    }

    MaterialAsset material{};
    try
    {
      const auto json = nlohmann::json::parse(bytes.begin(), bytes.end());
      if (!json.contains("vert") || !json.contains("frag"))
      {
        std::cerr << "AssetDatabase: material missing vert/frag: " << logicalPath << '\n';
        return kInvalidAsset;
      }
      material.vertPath = json["vert"].get<std::string>();
      material.fragPath = json["frag"].get<std::string>();
      if (json.contains("albedo") && json["albedo"].is_array() && json["albedo"].size() == 3)
      {
        material.albedo = Vec3{
            json["albedo"][0].get<float>(),
            json["albedo"][1].get<float>(),
            json["albedo"][2].get<float>()};
      }
      if (json.contains("albedoTexture") && json["albedoTexture"].is_string())
      {
        material.albedoTexturePath = json["albedoTexture"].get<std::string>();
      }
    }
    catch (const std::exception &ex)
    {
      std::cerr << "AssetDatabase: invalid material JSON " << logicalPath << ": " << ex.what() << '\n';
      return kInvalidAsset;
    }

    registerMaterialAsset(logicalPath, std::move(material));
    return guidForPath(logicalPath);
  }

  const MeshAsset *AssetDatabase::getMeshAsset(const AssetGuid guid) const
  {
    const auto it = m_meshes.find(guid);
    return it != m_meshes.end() ? &it->second : nullptr;
  }

  const MaterialAsset *AssetDatabase::getMaterialAsset(const AssetGuid guid) const
  {
    const auto it = m_materials.find(guid);
    return it != m_materials.end() ? &it->second : nullptr;
  }

  std::string_view AssetDatabase::pathForGuid(const AssetGuid guid) const
  {
    const auto it = m_guidToPath.find(guid);
    return it != m_guidToPath.end() ? it->second : std::string_view{};
  }

  AssetGuid AssetDatabase::guidForPath(const std::string_view logicalPath) const
  {
    const std::string key(logicalPath);
    const auto it = m_pathToGuid.find(key);
    return it != m_pathToGuid.end() ? it->second : kInvalidAsset;
  }

  void AssetDatabase::registerMeshAsset(const std::string_view logicalPath, MeshAsset asset)
  {
    const AssetGuid guid = assetGuidFromPath(logicalPath);
    const std::string path(logicalPath);
    m_pathToGuid[path] = guid;
    m_guidToPath[guid] = path;
    m_meshes[guid] = std::move(asset);
    m_nextGuid = std::max(m_nextGuid, guid + 1);
  }

  void AssetDatabase::registerMaterialAsset(const std::string_view logicalPath, MaterialAsset asset)
  {
    const AssetGuid guid = assetGuidFromPath(logicalPath);
    const std::string path(logicalPath);
    m_pathToGuid[path] = guid;
    m_guidToPath[guid] = path;
    m_materials[guid] = std::move(asset);
    m_nextGuid = std::max(m_nextGuid, guid + 1);
  }
}
