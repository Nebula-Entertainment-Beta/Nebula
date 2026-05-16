#include "texture.h"
#include "imageDecoder.h"

namespace Nebula
{
  std::shared_ptr<Texture> Texture::createFromFile(const IAssetProvider &assets, std::string_view path)
  {
    ImageData img;
    if (!ImageDecoder::decodeFromProvider(assets, path, img))
      return nullptr;
    return Texture::create(img.width, img.height, img.rgba.data());
  }
}