#pragma once

#include <string>

#include "renderer_api_type.h"

namespace Nebula
{
    /** Platform window creation options (title, size, graphics API). */
    struct WindowSpec
    {
        std::string title = "Nebula";
        int width = 1280;
        int height = 720;
        RendererAPIType api = RendererAPIType::OpenGL;
    };
}
