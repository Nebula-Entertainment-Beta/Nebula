#include "mesh_importer.h"

#include "mesh_asset.h"

#include <nlohmann/json.hpp>

#include <cstdlib>
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
    std::vector<uint8_t> bytes;
    if (!assets.readFile(logicalPath, bytes))
    {
      std::cerr << "importMeshObj: failed to read " << logicalPath << '\n';
      return false;
    }

    out = MeshAsset{};
    std::istringstream stream(std::string(reinterpret_cast<const char *>(bytes.data()), bytes.size()));
    std::string line;
    std::vector<MeshVertex> positions;
    std::vector<MeshVertex> texCoords;

    while (std::getline(stream, line))
    {
      if (line.empty() || line[0] == '#')
      {
        continue;
      }

      std::istringstream lineStream(line);
      std::string tag;
      lineStream >> tag;
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
            const int uvIndex =
                parseIndex(corner.texCoord, static_cast<int>(texCoords.size()));
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
      std::cerr << "importMeshObj: no geometry in " << logicalPath << '\n';
      return false;
    }

    computeMeshBounds(out);
    return true;
  }

  bool importMeshAsset(const IAssetProvider &assets, std::string_view logicalPath, MeshAsset &out)
  {
    if (endsWith(logicalPath, ".obj"))
    {
      return importMeshObj(assets, logicalPath, out);
    }

    if (endsWith(logicalPath, ".mesh"))
    {
      std::vector<uint8_t> bytes;
      if (!assets.readFile(logicalPath, bytes))
      {
        std::cerr << "importMeshAsset: failed to read " << logicalPath << '\n';
        return false;
      }

      try
      {
        const auto json = nlohmann::json::parse(bytes.begin(), bytes.end());
        if (!json.contains("source") || !json["source"].is_string())
        {
          std::cerr << "importMeshAsset: .mesh missing source: " << logicalPath << '\n';
          return false;
        }
        return importMeshAsset(assets, json["source"].get<std::string>(), out);
      }
      catch (const std::exception &ex)
      {
        std::cerr << "importMeshAsset: invalid .mesh JSON " << logicalPath << ": " << ex.what() << '\n';
        return false;
      }
    }

    return importMeshObj(assets, logicalPath, out);
  }
}
