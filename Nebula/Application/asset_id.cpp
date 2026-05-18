#include "asset_id.h"

#include <cctype>
#include <string>

namespace Nebula
{
  namespace
  {
    std::string normalizeAssetPath(std::string_view path)
    {
      std::string out(path);
      for (char &c : out)
      {
        if (c == '\\')
        {
          c = '/';
        }
        c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
      }
      while (!out.empty() && out.front() == '/')
      {
        out.erase(out.begin());
      }
      return out;
    }
  }

  AssetGuid assetGuidFromPath(std::string_view logicalPath)
  {
    const std::string normalized = normalizeAssetPath(logicalPath);
    constexpr uint64_t offset = 14695981039346656037ull;
    constexpr uint64_t prime = 1099511628211ull;
    uint64_t hash = offset;
    for (unsigned char c : normalized)
    {
      hash ^= c;
      hash *= prime;
    }
    return hash == kInvalidAsset ? 1ull : hash;
  }
}
