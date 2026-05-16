#pragma once
#include <cstdint>
#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

namespace Nebula
{

  class IAssetProvider
  {
  public:
    virtual ~IAssetProvider() = default;
    virtual bool readFile(std::string_view logicalPath, std::vector<uint8_t> &out) const = 0;
    virtual bool writeFile(std::string_view logicalPath, const std::vector<uint8_t> &data) const = 0;
  };

  class FileAssetProvider : public IAssetProvider
  {
  public:
    FileAssetProvider(); // resolves root via env / CMake define / exe fallbacks
    explicit FileAssetProvider(std::filesystem::path assetRoot);

    bool readFile(std::string_view logicalPath, std::vector<uint8_t> &out) const override;

    const std::vector<std::filesystem::path> &searchRoots() const { return m_searchRoots; }
    bool writeFile(std::string_view logicalPath, const std::vector<uint8_t> &data) const override;

  private:
    std::filesystem::path resolveExisting(std::string_view logicalPath) const;
    std::filesystem::path resolveForWrite(std::string_view logicalPath) const;
    static std::filesystem::path normalizeLogicalPath(std::string_view logicalPath);
    static std::filesystem::path resolveDefaultAssetRoot();

    std::vector<std::filesystem::path> m_searchRoots;
  };

} // namespace Nebula