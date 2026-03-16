#include "renderAPI.h"
#include "../openGL/openGL_Renderer.h"


namespace Nebula
{
    std::unique_ptr<RenderAPI> createrendererAPI()
    {
        // Here you can decide which RenderAPI implementation to create based on your needs
        // For example, you might want to check for the platform or user settings
        return std::make_unique<OpenGL_Renderer>();
    }
}