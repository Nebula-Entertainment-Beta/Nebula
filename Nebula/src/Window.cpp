/**
 * @file Window.cpp
 * @brief GLFW platform window; graphics API context comes from `detail::createGraphicsContext`.
 */
#include "Window.h"
#include "detail/graphics_context_factory.h"
#include "detail/window_native.h"
#include <GLFW/glfw3.h>
#include <iostream>
#include <cassert>

namespace Nebula
{

    struct WindowImpl
    {
        GLFWwindow *m_window = nullptr;
        bool m_glfwInitialized = false;
        bool m_glReady = false;
        std::unique_ptr<graphicsContext> m_graphicsContext;
    };

    Window::Window(const WindowSpec &spec)
        : m_impl(std::make_unique<WindowImpl>())
    {
        if (!glfwInit())
        {
            std::cerr << "Failed to initialize GLFW" << std::endl;
            return;
        }
        m_impl->m_glfwInitialized = true;

        detail::configureGlfwWindowHints(spec.api);

        m_impl->m_window =
            glfwCreateWindow(spec.width, spec.height, spec.title.c_str(), nullptr, nullptr);
        if (!m_impl->m_window)
        {
            std::cerr << "Failed to create GLFW window" << std::endl;
            glfwTerminate();
            m_impl->m_glfwInitialized = false;
            return;
        }

        m_impl->m_graphicsContext = detail::createGraphicsContext(m_impl->m_window, spec.api);
        m_impl->m_glReady =
            m_impl->m_graphicsContext != nullptr && m_impl->m_graphicsContext->isValid();
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
