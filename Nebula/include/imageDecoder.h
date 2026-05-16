#pragma once
#include "vector"
#include "assetProvider.h"

namespace Nebula
{

  struct ImageData
  {
    int width = 0;
    int height = 0;
    std::vector<uint8_t> rgba; // width * height * 4
  };

  class ImageDecoder
  {
  public:
    static bool decodeFromMemory(const uint8_t *data, size_t size, ImageData &out);
    static bool decodeFromProvider(const IAssetProvider &assets, std::string_view logicalPath, ImageData &out);
  };

}