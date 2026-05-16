#include "assetProvider.h"

#include <cstdlib>
#include <fstream>
#include <system_error>

#if defined(__APPLE__)
#include <mach-o/dyld.h>
#elif defined(_WIN32)
#include <windows.h>
#else
#include <unistd.h>
#include <limits.h>
#endif

namespace Nebula
{
  namespace
  {

    std::filesystem::path executablePath()
    {
#if defined(__APPLE__)
      uint32_t size = 0;
      _NSGetExecutablePath(nullptr, &size);
      std::string buf(size, '\0');
      if (_NSGetExecutablePath(buf.data(), &size) != 0)
        return {};
      return std::filesystem::path(buf.c_str());
#elif defined(_WIN32)
      wchar_t buf[MAX_PATH]{};
      const DWORD n = GetModuleFileNameW(nullptr, buf, MAX_PATH);
      if (n == 0 || n == MAX_PATH)
        return {};
      return std::filesystem::path(buf);
#else
      char buf[PATH_MAX]{};
      const ssize_t n = readlink("/proc/self/exe", buf, sizeof(buf) - 1);
      if (n <= 0)
        return {};
      buf[static_cast<size_t>(n)] = '\0';
      return std::filesystem::path(buf);
#endif
    }

    std::filesystem::path weaklyCanonicalOrAbsolute(const std::filesystem::path &p)
    {
      std::error_code ec;
      const auto c = std::filesystem::weakly_canonical(p, ec);
      return ec ? std::filesystem::absolute(p) : c;
    }

  } // namespace

  std::filesystem::path FileAssetProvider::normalizeLogicalPath(std::string_view logicalPath)
  {
    std::filesystem::path p = std::filesystem::path(logicalPath).lexically_normal();
    std::string s = p.generic_string();
    while (!s.empty() && s.front() == '/')
      s.erase(s.begin());
    if (s.empty())
      return {};
    p = std::filesystem::path(s);
    for (const auto &part : p)
    {
      if (part == "..")
        return {};
    }
    return p;
  }

  std::filesystem::path FileAssetProvider::resolveDefaultAssetRoot()
  {
    if (const char *env = std::getenv("NEBULA_ASSET_ROOT"))
    {
      if (env[0] != '\0')
        return weaklyCanonicalOrAbsolute(std::filesystem::path(env));
    }

#ifdef NEBULA_ASSET_ROOT
    return weaklyCanonicalOrAbsolute(std::filesystem::path(NEBULA_ASSET_ROOT));
#endif

    const std::filesystem::path exe = executablePath();
    if (!exe.empty())
    {
      const std::filesystem::path dir = exe.parent_path();
      const std::filesystem::path candidates[] = {
          dir / "assets",
          dir / ".." / "assets",
          dir / ".." / ".." / "assets", // build/Nimbus/Nimbus -> repo/assets
      };
      for (const auto &c : candidates)
      {
        std::error_code ec;
        if (std::filesystem::is_directory(c, ec) && !ec)
          return weaklyCanonicalOrAbsolute(c);
      }
    }

    return weaklyCanonicalOrAbsolute(std::filesystem::current_path() / "assets");
  }

  FileAssetProvider::FileAssetProvider()
      : FileAssetProvider(resolveDefaultAssetRoot())
  {
  }

  FileAssetProvider::FileAssetProvider(std::filesystem::path assetRoot)
  {
    m_searchRoots = {weaklyCanonicalOrAbsolute(std::move(assetRoot))};
  }

  std::filesystem::path FileAssetProvider::resolveExisting(std::string_view logicalPath) const
  {
    const std::filesystem::path rel = normalizeLogicalPath(logicalPath);
    if (rel.empty())
      return {};

    for (const auto &root : m_searchRoots)
    {
      const std::filesystem::path candidate = root / rel;
      std::error_code ec;
      if (std::filesystem::is_regular_file(candidate, ec) && !ec)
        return candidate;
    }
    return {};
  }

  std::filesystem::path FileAssetProvider::resolveForWrite(std::string_view logicalPath) const
  {
    const std::filesystem::path rel = normalizeLogicalPath(logicalPath);
    if (rel.empty() || m_searchRoots.empty())
      return {};
    return m_searchRoots.front() / rel;
  }

  bool FileAssetProvider::readFile(std::string_view logicalPath, std::vector<uint8_t> &out) const
  {
    const std::filesystem::path path = resolveExisting(logicalPath);
    if (path.empty())
      return false;

    std::error_code ec;
    const auto size = std::filesystem::file_size(path, ec);
    if (ec)
      return false;

    std::ifstream in(path, std::ios::binary);
    if (!in)
      return false;

    out.resize(static_cast<size_t>(size));
    if (size > 0 && !in.read(reinterpret_cast<char *>(out.data()), static_cast<std::streamsize>(size)))
      return false;
    return true;
  }

  bool FileAssetProvider::writeFile(std::string_view logicalPath, const std::vector<uint8_t> &data) const
  {
    const std::filesystem::path path = resolveForWrite(logicalPath);
    if (path.empty())
      return false;

    std::error_code ec;
    std::filesystem::create_directories(path.parent_path(), ec);
    if (ec)
      return false;

    std::ofstream out(path, std::ios::binary | std::ios::trunc);
    if (!out)
      return false;

    if (!data.empty())
      out.write(reinterpret_cast<const char *>(data.data()), static_cast<std::streamsize>(data.size()));
    return static_cast<bool>(out);
  }

} // namespace Nebula