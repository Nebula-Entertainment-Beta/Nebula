#include "renderBackend.h"

namespace Nebula
{

    RenderBackend createRenderBackend(RendererAPIType api)
    {
        switch (api)
        {
        case RendererAPIType::OpenGL:
            return createOpenGLRenderBackend();
        case RendererAPIType::Vulkan:
            return createVulkanRenderBackend();
        case RendererAPIType::None:
        default:
            return RenderBackend{};
        }
    }

}
