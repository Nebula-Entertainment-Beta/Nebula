#include "imageDecoder.h"

#include <cstddef>
#include <limits>
#include <vector>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

namespace Nebula
{

  bool ImageDecoder::decodeFromMemory(const uint8_t *data, size_t size, ImageData &out)
  {
    out = ImageData{};

    if (!data || size == 0)
    {
      return false;
    }

    if (size > static_cast<size_t>(std::numeric_limits<int>::max()))
    {
      return false;
    }

    int width = 0;
    int height = 0;
    int channels = 0;

    stbi_uc *pixels = stbi_load_from_memory(
        data,
        static_cast<int>(size),
        &width,
        &height,
        &channels,
        STBI_rgb_alpha);

    if (!pixels || width <= 0 || height <= 0)
    {
      if (pixels)
      {
        stbi_image_free(pixels);
      }
      return false;
    }

    const size_t byteCount =
        static_cast<size_t>(width) * static_cast<size_t>(height) * 4u;

    out.width = width;
    out.height = height;
    out.rgba.assign(pixels, pixels + byteCount);

    stbi_image_free(pixels);
    return true;
  }

  bool ImageDecoder::decodeFromProvider(
      const IAssetProvider &assets,
      std::string_view logicalPath,
      ImageData &out)
  {
    out = ImageData{};

    std::vector<uint8_t> bytes;
    if (!assets.readFile(logicalPath, bytes))
    {
      return false;
    }

    return decodeFromMemory(bytes.data(), bytes.size(), out);
  }

} // namespace Nebula