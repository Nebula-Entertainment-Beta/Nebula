/**
 * @file renderAPI.cpp
 * @brief Global API selection and construction of the OpenGL renderer backend.
 */
#include "renderAPI.h"
#include "openGL_RenderBackends.h"

namespace Nebula
{
    RendererAPIType RenderAPI::s_API = RendererAPIType::OpenGL;
    RendererAPIType RenderAPI::getAPI()
    {
        return s_API;
    }
    void RenderAPI::setAPI(RendererAPIType api)
    {
        s_API = api;
    }
    std::unique_ptr<RenderAPI> createRendererAPI(RendererAPIType api)
    {
        switch (api)
        {
        case RendererAPIType::OpenGL:
            RenderAPI::setAPI(RendererAPIType::OpenGL);
            return createOpenGLRendererAPI();
        case RendererAPIType::None:
        default:
            return nullptr;
        }
    }

}