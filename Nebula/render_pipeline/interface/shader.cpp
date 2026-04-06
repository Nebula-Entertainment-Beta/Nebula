
#include "shader.h"
#include "renderAPI.h"
#include "../openGL/openGL_Shader.h"



namespace Nebula
{
    

    std::shared_ptr<Shader> Shader::create(const std::string& vertexSrc, const std::string& fragmentSrc)
    {
      switch(RenderAPI::getAPI()){
        case RendererAPIType::OpenGL:
          return OpenGL_Shader::create(vertexSrc, fragmentSrc);
        default:
          return nullptr;
      }
    }
}