#include "texture.h"
#include "renderAPI.h"
#include "../openGL/openGL_texture.h"

namespace Nebula{
  std::shared_ptr<Texture> Texture::create(int width, int height, const void* rgba){
    switch (RenderAPI::getAPI()){
      case RendererAPIType::OpenGL:
        return OpenGL_Texture::create(width, height, rgba);
      default:
        return nullptr;
    }
  }
}

