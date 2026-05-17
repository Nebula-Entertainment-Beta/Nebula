#include "assetManager.h"
#include "builtinMeshes.h"

#include <iostream>

namespace Nebula
{
  // namespace

  void AssetManager::loadBuiltins(const IAssetProvider &fileAssets)
  {
    if (m_builtinsLoaded)
      return;
    loadBuiltinMeshes();
    loadBuiltinMaterials(fileAssets);
    m_builtinsLoaded = true;
  }

  bool loadTextFile(const IAssetProvider &assets, std::string_view logicalPath, std::string &out)
  {
    std::vector<uint8_t> bytes;
    if (!assets.readFile(logicalPath, bytes))
      return false;
    out.assign(reinterpret_cast<const char *>(bytes.data()), bytes.size());
    return true;
  }

  void AssetManager::loadBuiltinMeshes()
  {
    m_meshes.resize(2);
    if (!buildBuiltinCubeMesh(m_meshes[kBuiltinMeshCube]))
      std::cerr << "Failed to build builtin cube mesh.\n";
    if (!buildBuiltinGroundMesh(m_meshes[kBuiltinMeshGround]))
      std::cerr << "Failed to build builtin ground mesh.\n";
  }

  void AssetManager::loadBuiltinMaterials(const IAssetProvider &fileAssets)
  {
    std::string vertSrc, fragSrc;
    if (!loadTextFile(fileAssets, "shaders/solid_color.vert", vertSrc) ||
        !loadTextFile(fileAssets, "shaders/solid_color.frag", fragSrc))
    {
      std::cerr << "Failed to load solid_color shaders from assets.\n";
      return;
    }

    auto shader = Shader::create(vertSrc, fragSrc);
    if (!shader)
    {
      std::cerr << "Builtin Shader::create failed.\n";
      return;
    }

    m_materials.resize(2);
    m_materials[kBuiltinMaterialGround] = Material{
        "shaders/solid_color.vert",
        "shaders/solid_color.frag",
        shader,
        Vec3{0.85f, 0.12f, 0.1f}};
    m_materials[kBuiltinMaterialCube] = Material{
        "shaders/solid_color.vert",
        "shaders/solid_color.frag",
        shader,
        Vec3{0.15f, 0.45f, 0.95f}};
  }

  const Mesh *AssetManager::getMesh(MeshHandle handle) const
  {
    if (!isMeshValid(handle))
      return nullptr;
    return &m_meshes[handle];
  }

  const Material *AssetManager::getMaterial(MaterialHandle handle) const
  {
    if (!isMaterialValid(handle))
      return nullptr;
    return &m_materials[handle];
  }

  bool AssetManager::isMeshValid(MeshHandle handle) const
  {
    return handle < m_meshes.size() && m_meshes[handle].vao && m_meshes[handle].indexCount > 0;
  }

  bool AssetManager::isMaterialValid(MaterialHandle handle) const
  {
    return handle < m_materials.size() && m_materials[handle].shader != nullptr;
  }

} // namespace Nebula