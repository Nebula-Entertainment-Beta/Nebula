/**
 * @file Window.cpp
 * @brief GLFW window + OpenGL 4.1 core context (forward-compatible on macOS).
 */
#include <glad/glad.h>
#include "Window.h"
#include "detail/window_native.h"
#include "detail/openGL_GraphicsContext.h"
#include <GLFW/glfw3.h>
#include <iostream>
#include <string_view>
#include <cassert>

namespace Nebula
{

    struct WindowImpl
    {
        GLFWwindow *m_window = nullptr;
        bool m_glfwInitialized = false;
        bool m_glReady = false;
        std::unique_ptr<detail::OpenGLGraphicsContext> m_graphicsContext;
    };

    Window::Window(std::string_view title, int width, int height)
        : m_impl(std::make_unique<WindowImpl>())
    {
        if (!glfwInit())
        {
            std::cerr << "Failed to initialize GLFW" << std::endl;
            return;
        }
        m_impl->m_glfwInitialized = true;

        // Request a context that matches generated GLAD (gl=4.1). Required for core + macOS.
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

        m_impl->m_window = glfwCreateWindow(width, height, title.data(), nullptr, nullptr);
        if (!m_impl->m_window)
        {
            std::cerr << "Failed to create GLFW window" << std::endl;
            glfwTerminate();
            m_impl->m_glfwInitialized = false;
            return;
        }

        glfwMakeContextCurrent(m_impl->m_window);

        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
        {
            std::cerr << "Failed to initialize GLAD (OpenGL loader). "
                         "GPU/driver may not support the requested context.\n";
            glfwDestroyWindow(m_impl->m_window);
            m_impl->m_window = nullptr;
            glfwTerminate();
            m_impl->m_glfwInitialized = false;
            return;
        }
        m_impl->m_graphicsContext = std::make_unique<detail::OpenGLGraphicsContext>(m_impl->m_window);
        m_impl->m_glReady = true;
    }

    namespace detail
    {
        void *nativeWindowHandleForInput(const Window &window)
        {
            return window.m_impl ? static_cast<void *>(window.m_impl->m_window) : nullptr;
        }
    } // namespace detail

    bool Window::shouldWindowClose()
    {
        if (!m_impl || !m_impl->m_window)
            return true;
        return glfwWindowShouldClose(m_impl->m_window) != 0;
    }

    void Window::getFramebufferSize(int &outWidth, int &outHeight) const
    {
        if (m_impl && m_impl->m_graphicsContext)
        {
            m_impl->m_graphicsContext->getFramebufferSize(outWidth, outHeight);
        }

        else
        {
            outWidth = 0;
            outHeight = 0;
        }
    }

    void Window::pollEvents()
    {
        if (!m_impl || !m_impl->m_window)
        {
            std::cerr << "Failed to poll events for GLFW window" << std::endl;
            return;
        }
        glfwPollEvents();
    }

    void Window::swapBuffers()
    {

        if (m_impl && m_impl->m_graphicsContext)
        {

            m_impl->m_graphicsContext->swap();
        }
    }

    Window::~Window()
    {
        if (!m_impl)
            return;
        if (m_impl->m_window)
        {
            glfwDestroyWindow(m_impl->m_window);
            m_impl->m_window = nullptr;
        }
        if (m_impl->m_glfwInitialized)
        {
            glfwTerminate();
            m_impl->m_glfwInitialized = false;
        }
    }

    bool Window::isValid() const
    {
        return m_impl && m_impl->m_window != nullptr && m_impl->m_glReady;
    }

    void Window::setCursorMode(CursorMode mode)
    {
        if (!m_impl || !m_impl->m_window)
            return;

        int glfwCursorMode = GLFW_CURSOR_NORMAL;
        switch (mode)
        {
        case CursorMode::Normal:
            glfwCursorMode = GLFW_CURSOR_NORMAL;
            break;
        case CursorMode::Hidden:
            glfwCursorMode = GLFW_CURSOR_HIDDEN;
            break;
        case CursorMode::Disabled:
            glfwCursorMode = GLFW_CURSOR_DISABLED;
            break;
        }
        glfwSetInputMode(m_impl->m_window, GLFW_CURSOR, glfwCursorMode);
    }

    void Window::setRawMouseMotion(bool enabled)
    {
        if (!m_impl || !m_impl->m_window)
            return;

#ifdef GLFW_RAW_MOUSE_MOTION
        if (!glfwRawMouseMotionSupported())
            return;
        glfwSetInputMode(m_impl->m_window, GLFW_RAW_MOUSE_MOTION, enabled ? GLFW_TRUE : GLFW_FALSE);
#else
        (void)enabled;
#endif
    }

    const graphicsContext &Window::getGraphicsContext() const
    {
        assert(m_impl != nullptr && m_impl->m_graphicsContext != nullptr);
        return *m_impl->m_graphicsContext;
    }

    graphicsContext &Window::getGraphicsContext()
    {
        assert(m_impl != nullptr && m_impl->m_graphicsContext != nullptr);
        return *m_impl->m_graphicsContext;
    }

} // namespace Nebula
