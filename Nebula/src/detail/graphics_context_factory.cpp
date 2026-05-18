#include <glad/glad.h>

#include "graphics_context_factory.h"

#include "openGL_GraphicsContext.h"
#include "vulkan_GraphicsContext.h"

#include <GLFW/glfw3.h>
#include <iostream>

namespace Nebula::detail
{

    void configureGlfwWindowHints(RendererAPIType api)
    {
        glfwDefaultWindowHints();
        switch (api)
        {
        case RendererAPIType::OpenGL:
            glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
            glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
            glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
            glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
            break;
        case RendererAPIType::Vulkan:
            glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
            break;
        case RendererAPIType::None:
        default:
            break;
        }
    }

    std::unique_ptr<graphicsContext> createGraphicsContext(GLFWwindow *nativeWindow,
                                                           RendererAPIType api)
    {
        if (!nativeWindow)
        {
            return nullptr;
        }

        switch (api)
        {
        case RendererAPIType::OpenGL:
        {
            glfwMakeContextCurrent(nativeWindow);
            if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
            {
                std::cerr << "Failed to initialize GLAD (OpenGL loader). "
                             "GPU/driver may not support the requested context.\n";
                return nullptr;
            }
            return std::make_unique<OpenGLGraphicsContext>(nativeWindow);
        }
        case RendererAPIType::Vulkan:
            return std::make_unique<VulkanGraphicsContext>(nativeWindow);
        case RendererAPIType::None:
        default:
            return nullptr;
        }
    }

}
