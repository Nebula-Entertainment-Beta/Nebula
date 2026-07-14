#include "mesh_importer.h"

#include "mesh_asset.h"
#include "mesh_import_options.h"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <nlohmann/json.hpp>

#include <algorithm>
#include <cctype>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace Nebula
{
  namespace
  {
    bool endsWith(std::string_view value, std::string_view suffix)
    {
      return value.size() >= suffix.size() &&
             value.compare(value.size() - suffix.size(), suffix.size(), suffix) == 0;
    }

    bool isAssimpSourcePath(std::string_view path)
    {
      return endsWith(path, ".fbx") || endsWith(path, ".obj") || endsWith(path, ".gltf") ||
             endsWith(path, ".glb") || endsWith(path, ".dae");
    }

    const FileAssetProvider *asFileProvider(const IAssetProvider &assets)
    {
      return dynamic_cast<const FileAssetProvider *>(&assets);
    }

    MeshVertex vertexFromAssimp(const aiVector3D &p, const aiVector3D &uv)
    {
      MeshVertex v{};
      v.x = p.x;
      v.y = p.y;
      v.z = p.z;
      v.u = uv.x;
      v.v = uv.y;
      return v;
    }

    bool meshMatchesSelection(const aiMesh *mesh, const MeshImportOptions &options, int index)
    {
      if (!options.meshName.empty())
      {
        return mesh->mName.C_Str() == options.meshName;
      }
      if (options.meshIndex >= 0)
      {
        return index == options.meshIndex;
      }
      return true;
    }

    bool appendAssimpMesh(const aiMesh *mesh, MeshAsset &out)
    {
      if (mesh == nullptr || mesh->mNumVertices == 0)
      {
        return false;
      }

      const uint32_t baseVertex = static_cast<uint32_t>(out.vertices.size());
      for (unsigned int v = 0; v < mesh->mNumVertices; ++v)
      {
        const aiVector3D uv = mesh->HasTextureCoords(0) ? mesh->mTextureCoords[0][v] : aiVector3D{};
        out.vertices.push_back(vertexFromAssimp(mesh->mVertices[v], uv));
      }

      for (unsigned int f = 0; f < mesh->mNumFaces; ++f)
      {
        const aiFace &face = mesh->mFaces[f];
        if (face.mNumIndices < 3)
        {
          continue;
        }
        for (unsigned int t = 1; t + 1 < face.mNumIndices; ++t)
        {
          out.indices.push_back(baseVertex + face.mIndices[0]);
          out.indices.push_back(baseVertex + face.mIndices[t]);
          out.indices.push_back(baseVertex + face.mIndices[t + 1]);
        }
      }
      return true;
    }

    bool importMeshAssimpFile(const std::filesystem::path &physicalPath, const MeshImportOptions &options,
                              MeshAsset &out)
    {
      Assimp::Importer importer;
      const unsigned int flags = aiProcess_Triangulate | aiProcess_GenNormals |
                                 aiProcess_JoinIdenticalVertices | aiProcess_ImproveCacheLocality;
      const aiScene *scene = importer.ReadFile(physicalPath.string(), flags);
      if (scene == nullptr || scene->mNumMeshes == 0)
      {
        std::cerr << "importMeshAssimpFile: failed " << physicalPath << ": "
                  << importer.GetErrorString() << '\n';
        return false;
      }

      out = MeshAsset{};
      bool added = false;
      for (unsigned int i = 0; i < scene->mNumMeshes; ++i)
      {
        const aiMesh *mesh = scene->mMeshes[i];
        if (!meshMatchesSelection(mesh, options, static_cast<int>(i)))
        {
          continue;
        }
        added |= appendAssimpMesh(mesh, out);
        if (options.meshIndex >= 0 || !options.meshName.empty())
        {
          break;
        }
      }

      if (!added || out.vertices.empty() || out.indices.empty())
      {
        std::cerr << "importMeshAssimpFile: no matching geometry in " << physicalPath << '\n';
        if (scene->mNumMeshes > 0)
        {
          std::cerr << "  available submeshes:\n";
          for (unsigned int i = 0; i < scene->mNumMeshes; ++i)
          {
            std::cerr << "    [" << i << "] " << scene->mMeshes[i]->mName.C_Str() << '\n';
          }
        }
        return false;
      }

      applyMeshImportPostProcess(out, options);
      return true;
    }

    int parseIndex(int raw, int count)
    {
      if (raw > 0)
      {
        return raw - 1;
      }
      return count + raw;
    }

    struct ObjCorner
    {
      int position = -1;
      int texCoord = -1;
    };

    bool parseObjCorner(const std::string &corner, ObjCorner &out)
    {
      out = {};
      const size_t firstSlash = corner.find('/');
      if (firstSlash == std::string::npos)
      {
        out.position = std::stoi(corner);
        return true;
      }

      out.position = std::stoi(corner.substr(0, firstSlash));
      const size_t secondSlash = corner.find('/', firstSlash + 1);
      if (secondSlash == std::string::npos)
      {
        const std::string tex = corner.substr(firstSlash + 1);
        if (!tex.empty())
        {
          out.texCoord = std::stoi(tex);
        }
        return true;
      }

      const std::string tex = corner.substr(firstSlash + 1, secondSlash - firstSlash - 1);
      if (!tex.empty())
      {
        out.texCoord = std::stoi(tex);
      }
      return true;
    }
  }

  bool importMeshObj(const IAssetProvider &assets, std::string_view logicalPath, MeshAsset &out)
  {
    MeshImportOptions options{};
    options.sourcePath = std::string(logicalPath);
    return importMeshWithOptions(assets, options, out);
  }

  bool importMeshWithOptions(const IAssetProvider &assets, const MeshImportOptions &options, MeshAsset &out)
  {
    const std::string_view source = options.sourcePath;
    if (const FileAssetProvider *files = asFileProvider(assets))
    {
      if (isAssimpSourcePath(source))
      {
        const std::filesystem::path physical = files->resolvePhysicalPath(source);
        if (!physical.empty())
        {
          return importMeshAssimpFile(physical, options, out);
        }
      }
    }

    std::vector<uint8_t> bytes;
    if (!assets.readFile(source, bytes))
    {
      std::cerr << "importMeshWithOptions: failed to read " << source << '\n';
      return false;
    }

    out = MeshAsset{};
    std::istringstream stream(std::string(reinterpret_cast<const char *>(bytes.data()), bytes.size()));
    std::string line;
    std::vector<MeshVertex> positions;
    std::vector<MeshVertex> texCoords;
    std::string currentObject;
    bool useObjectFilter = !options.meshName.empty();

    while (std::getline(stream, line))
    {
      if (line.empty() || line[0] == '#')
      {
        continue;
      }

      std::istringstream lineStream(line);
      std::string tag;
      lineStream >> tag;
      if (tag == "o" || tag == "g")
      {
        lineStream >> currentObject;
        continue;
      }
      if (tag == "v")
      {
        MeshVertex v{};
        lineStream >> v.x >> v.y >> v.z;
        positions.push_back(v);
      }
      else if (tag == "vt")
      {
        MeshVertex uv{};
        lineStream >> uv.u >> uv.v;
        texCoords.push_back(uv);
      }
      else if (tag == "f")
      {
        if (useObjectFilter && currentObject != options.meshName)
        {
          continue;
        }

        std::vector<ObjCorner> face;
        std::string corner;
        while (lineStream >> corner)
        {
          ObjCorner parsed{};
          if (!parseObjCorner(corner, parsed))
          {
            continue;
          }
          face.push_back(parsed);
        }

        if (face.size() < 3)
        {
          continue;
        }

        const auto emitTriangle = [&](const ObjCorner &c0, const ObjCorner &c1, const ObjCorner &c2)
        {
          const int i0 = parseIndex(c0.position, static_cast<int>(positions.size()));
          const int i1 = parseIndex(c1.position, static_cast<int>(positions.size()));
          const int i2 = parseIndex(c2.position, static_cast<int>(positions.size()));
          if (i0 < 0 || i1 < 0 || i2 < 0 || i0 >= static_cast<int>(positions.size()) ||
              i1 >= static_cast<int>(positions.size()) || i2 >= static_cast<int>(positions.size()))
          {
            return;
          }

          const auto resolveUv = [&](const ObjCorner &corner, const MeshVertex &position) -> MeshVertex
          {
            MeshVertex v = position;
            if (corner.texCoord == -1 || texCoords.empty())
            {
              return v;
            }
            const int uvIndex = parseIndex(corner.texCoord, static_cast<int>(texCoords.size()));
            if (uvIndex >= 0 && uvIndex < static_cast<int>(texCoords.size()))
            {
              v.u = texCoords[static_cast<size_t>(uvIndex)].u;
              v.v = texCoords[static_cast<size_t>(uvIndex)].v;
            }
            return v;
          };

          out.indices.push_back(static_cast<uint32_t>(out.vertices.size()));
          out.vertices.push_back(resolveUv(c0, positions[static_cast<size_t>(i0)]));
          out.indices.push_back(static_cast<uint32_t>(out.vertices.size()));
          out.vertices.push_back(resolveUv(c1, positions[static_cast<size_t>(i1)]));
          out.indices.push_back(static_cast<uint32_t>(out.vertices.size()));
          out.vertices.push_back(resolveUv(c2, positions[static_cast<size_t>(i2)]));
        };

        for (size_t i = 1; i + 1 < face.size(); ++i)
        {
          emitTriangle(face[0], face[i], face[i + 1]);
        }
      }
    }

    if (out.vertices.empty() || out.indices.empty())
    {
      std::cerr << "importMeshWithOptions: no geometry in " << source << '\n';
      return false;
    }

    applyMeshImportPostProcess(out, options);
    return true;
  }

  bool importMeshAsset(const IAssetProvider &assets, std::string_view logicalPath, MeshAsset &out)
  {
    if (endsWith(logicalPath, ".mesh"))
    {
      std::vector<uint8_t> bytes;
      if (!assets.readFile(logicalPath, bytes))
      {
        std::cerr << "importMeshAsset: failed to read " << logicalPath << '\n';
        return false;
      }

      MeshImportOptions options{};
      if (!parseMeshDescriptorJson(
              std::string(reinterpret_cast<const char *>(bytes.data()), bytes.size()), options))
      {
        std::cerr << "importMeshAsset: invalid .mesh JSON " << logicalPath << '\n';
        return false;
      }
      return importMeshWithOptions(assets, options, out);
    }

    if (isAssimpSourcePath(logicalPath))
    {
      MeshImportOptions options{};
      options.sourcePath = std::string(logicalPath);
      if (endsWith(logicalPath, ".fbx"))
      {
        options.importScale = 0.01f;
      }
      return importMeshWithOptions(assets, options, out);
    }

    return importMeshObj(assets, logicalPath, out);
  }
}
