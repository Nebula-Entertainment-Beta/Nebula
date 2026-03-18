#include "renderAPI.h"
#include "../openGL/openGL_Renderer.h"


namespace Nebula
{
    RendererAPIType RenderAPI::s_API = RendererAPIType::OpenGL;
    RendererAPIType RenderAPI::getAPI(){
        return s_API;
    }
    std::unique_ptr<RenderAPI> createrendererAPI()
    {
        RenderAPI::s_API =RendererAPIType::OpenGL;
        return std::make_unique<OpenGL_Renderer>();

        
    }
}