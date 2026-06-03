#include "assetManager.h"

#include "builtin_assets.h"
#include "builtinMeshes.h"
#include "mesh_upload.h"
#include "shader.h"
#include "texture.h"

#include <iostream>

namespace Nebula
{
  namespace
  {
    bool loadTextFile(const IAssetProvider &assets, std::string_view logicalPath, std::string &out)
    {
      std::vector<uint8_t> bytes;
      if (!assets.readFile(logicalPath, bytes))
      {
        return false;
      }
      out.assign(reinterpret_cast<const char *>(bytes.data()), bytes.size());
      return true;
    }
  }

  AssetManager::AssetManager(const IAssetProvider &assets) : m_assets(assets), m_database(assets) {}

  void AssetManager::loadBuiltins(IRenderResourceFactory &resources)
  {
    if (m_builtinsLoaded)
    {
      return;
    }

    registerBuiltinCpuAssets();
    loadBuiltinGpuAssets(resources);
    m_builtinsLoaded = true;
  }

  void AssetManager::registerBuiltinCpuAssets()
  {
    MeshAsset cube{};
    MeshAsset ground{};

    if (buildBuiltinCubeMeshAsset(cube))
    {
      m_database.registerMeshAsset(kBuiltinMeshCubePath, std::move(cube));
    }
    if (buildBuiltinGroundMeshAsset(ground))
    {
      m_database.registerMeshAsset(kBuiltinMeshGroundPath, std::move(ground));
    }

    m_database.registerMaterialAsset(
        kBuiltinMaterialGroundPath,
        MaterialAsset{
            "shaders/solid_color.vert",
            "shaders/solid_color.frag",
            Vec3{0.85f, 0.12f, 0.1f},
            {}});
    m_database.registerMaterialAsset(
        kBuiltinMaterialCubePath,
        MaterialAsset{
            "shaders/solid_color.vert",
            "shaders/solid_color.frag",
            Vec3{0.15f, 0.45f, 0.95f},
            {}});

    m_database.registerMaterialAsset(
        kBuiltinMaterialEnemyPath,
        MaterialAsset{
            "shaders/solid_color.vert",
            "shaders/solid_color.frag",
            Vec3{0.0f, 1.0f, 0.0f}, // is color
            {}});

    m_database.registerMaterialAsset(
        kBuiltinMaterialPlayerPath,
        MaterialAsset{
            "shaders/solid_color.vert",
            "shaders/solid_color.frag",
            Vec3{1.0f, 0.0f, 0.0f}, // is color
            {}});
  }

  void AssetManager::loadBuiltinGpuAssets(IRenderResourceFactory &resources)
  {
    loadMesh(kBuiltinMeshCubePath, resources);
    loadMesh(kBuiltinMeshGroundPath, resources);
    loadMaterial(kBuiltinMaterialGroundPath, resources);
    loadMaterial(kBuiltinMaterialCubePath, resources);
    loadMaterial(kBuiltinMaterialEnemyPath, resources);
    loadMaterial(kBuiltinMaterialPlayerPath, resources);
  }

  void AssetManager::resolveScene(Scene &scene, IRenderResourceFactory &resources)
  {
    for (const Entity entity : scene.getAllEntities())
    {
      if (!scene.hasComponent<MeshRendererComponent>(entity))
      {
        continue;
      }

      auto &meshRenderer = scene.getComponent<MeshRendererComponent>(entity);
      if (!meshRenderer.m_meshPath.empty())
      {
        meshRenderer.m_meshID = loadMesh(meshRenderer.m_meshPath, resources);
      }
      if (!meshRenderer.m_materialPath.empty())
      {
        meshRenderer.m_materialID = loadMaterial(meshRenderer.m_materialPath, resources);
      }
    }
  }

  MeshHandle AssetManager::loadMesh(const AssetGuid guid, IRenderResourceFactory &resources)
  {
    if (guid == kInvalidAsset)
    {
      return kInvalidMesh;
    }

    if (const auto existing = m_meshGuidToHandle.find(guid); existing != m_meshGuidToHandle.end())
    {
      return existing->second;
    }

    const MeshAsset *asset = m_database.getMeshAsset(guid);
    if (!asset)
    {
      return kInvalidMesh;
    }

    Mesh gpuMesh{};
    if (!uploadMeshAsset(resources, *asset, gpuMesh))
    {
      return kInvalidMesh;
    }

    const MeshHandle handle = static_cast<MeshHandle>(m_meshes.size());
    m_meshes.push_back(std::move(gpuMesh));
    m_meshGuidToHandle[guid] = handle;
    m_meshHandleToGuid[handle] = guid;
    return handle;
  }

  MeshHandle AssetManager::loadMesh(const std::string_view logicalPath, IRenderResourceFactory &resources)
  {
    const AssetGuid guid = m_database.loadOrGetMesh(logicalPath);
    return loadMesh(guid, resources);
  }

  MaterialHandle AssetManager::loadMaterial(const AssetGuid guid, IRenderResourceFactory &resources)
  {
    if (guid == kInvalidAsset)
    {
      return kInvalidMaterial;
    }

    if (const auto existing = m_materialGuidToHandle.find(guid); existing != m_materialGuidToHandle.end())
    {
      return existing->second;
    }

    const MaterialAsset *asset = m_database.getMaterialAsset(guid);
    if (!asset)
    {
      return kInvalidMaterial;
    }

    std::string vertSrc;
    std::string fragSrc;
    if (!loadTextFile(m_assets, asset->vertPath, vertSrc) || !loadTextFile(m_assets, asset->fragPath, fragSrc))
    {
      std::cerr << "loadMaterial: failed to read shaders for guid " << guid << '\n';
      return kInvalidMaterial;
    }

    auto shader = resources.createShader(vertSrc, fragSrc);
    if (!shader)
    {
      std::cerr << "loadMaterial: shader creation failed for guid " << guid << '\n';
      return kInvalidMaterial;
    }

    std::shared_ptr<Texture> albedoTexture;
    if (!asset->albedoTexturePath.empty())
    {
      albedoTexture = Texture::createFromFile(resources, m_assets, asset->albedoTexturePath);
      if (!albedoTexture)
      {
        std::cerr << "loadMaterial: failed to load albedo texture for guid " << guid << '\n';
      }
    }

    const MaterialHandle handle = static_cast<MaterialHandle>(m_materials.size());
    m_materials.push_back(Material{
        asset->vertPath,
        asset->fragPath,
        std::move(shader),
        asset->albedo,
        std::move(albedoTexture)});
    m_materialGuidToHandle[guid] = handle;
    m_materialHandleToGuid[handle] = guid;
    return handle;
  }

  MaterialHandle AssetManager::loadMaterial(const std::string_view logicalPath, IRenderResourceFactory &resources)
  {
    const AssetGuid guid = m_database.loadOrGetMaterial(logicalPath);
    return loadMaterial(guid, resources);
  }

  void AssetManager::unloadMesh(const MeshHandle handle)
  {
    if (!isMeshValid(handle))
    {
      return;
    }

    const auto guidIt = m_meshHandleToGuid.find(handle);
    if (guidIt != m_meshHandleToGuid.end())
    {
      m_meshGuidToHandle.erase(guidIt->second);
      m_meshHandleToGuid.erase(guidIt);
    }

    m_meshes[handle] = Mesh{};
  }

  void AssetManager::unloadMaterial(const MaterialHandle handle)
  {
    if (!isMaterialValid(handle))
    {
      return;
    }

    const auto guidIt = m_materialHandleToGuid.find(handle);
    if (guidIt != m_materialHandleToGuid.end())
    {
      m_materialGuidToHandle.erase(guidIt->second);
      m_materialHandleToGuid.erase(guidIt);
    }

    m_materials[handle] = Material{};
  }

  const Mesh *AssetManager::getMesh(const MeshHandle handle) const
  {
    if (!isMeshValid(handle))
    {
      return nullptr;
    }
    return &m_meshes[handle];
  }

  const Material *AssetManager::getMaterial(const MaterialHandle handle) const
  {
    if (!isMaterialValid(handle))
    {
      return nullptr;
    }
    return &m_materials[handle];
  }

  bool AssetManager::isMeshValid(const MeshHandle handle) const
  {
    return handle < m_meshes.size() && m_meshes[handle].vao && m_meshes[handle].indexCount > 0;
  }

  bool AssetManager::isMaterialValid(const MaterialHandle handle) const
  {
    return handle < m_materials.size() && m_materials[handle].shader != nullptr;
  }

  std::string_view AssetManager::meshPathForHandle(const MeshHandle handle) const
  {
    const auto it = m_meshHandleToGuid.find(handle);
    if (it == m_meshHandleToGuid.end())
    {
      return {};
    }
    return m_database.pathForGuid(it->second);
  }

  std::string_view AssetManager::materialPathForHandle(const MaterialHandle handle) const
  {
    const auto it = m_materialHandleToGuid.find(handle);
    if (it == m_materialHandleToGuid.end())
    {
      return {};
    }
    return m_database.pathForGuid(it->second);
  }
}
