#pragma once

#include <memory>

#include "graphicsContext.h"
#include "renderer_api_type.h"

struct GLFWwindow;

namespace Nebula::detail
{
    void configureGlfwWindowHints(RendererAPIType api);

    std::unique_ptr<graphicsContext> createGraphicsContext(GLFWwindow *nativeWindow,
                                                            RendererAPIType api);
}
