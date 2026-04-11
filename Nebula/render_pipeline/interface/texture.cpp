#include "texture.h"
#include "renderAPI.h"
#include "../openGL/openGL_texture.h"

namespace Nebula{
  std::shared_ptr<Texture> Texture::create(int width, int height, const void* rgba){
    switch (RenderAPI::getAPI()){
      case RendererAPIType::OpenGL:
        return std::make_shared<OpenGL_Texture>(width, height, rgba);
      default:
        return nullptr;
    }
  }
}

