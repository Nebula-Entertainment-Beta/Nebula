/**
 * @file Window.h
 * @brief Creates an OS window (GLFW) and a graphics context for the chosen API.
 *
 * **Big picture:** Your game needs a drawable surface. GLFW opens the window; a factory in
 * `src/detail/` creates OpenGL or (later) Vulkan contexts. Only call rendering code if `isValid()`
 * is true — otherwise setup failed.
 */
#pragma once
#include <memory>
#include "graphicsContext.h"
#include "window_spec.h"

namespace Nebula
{
    class Window;

    namespace detail
    {
        void *nativeWindowHandleForInput(const Window &window);
    }

    enum class CursorMode
    {
        Normal,
        Hidden,
        Disabled,
    };
    struct WindowImpl;

    /** @brief Owns a native window and a `graphicsContext` for the API in `WindowSpec`. */
    class Window
    {
    public:
        /** @brief Opens a window and creates the graphics context described by `spec`. */
        explicit Window(const WindowSpec &spec);

        /** @return True when the user closed the window or the window failed. */
        bool shouldWindowClose();

        ~Window();

        /**
         * @brief Pixel size of the drawable framebuffer (may differ from window size on Hi-DPI).
         * Use this for `glViewport` and projection aspect, not the constructor width/height alone.
         */
        void getFramebufferSize(int &outWidth, int &outHeight) const;

        /**
         * @brief False if GLFW or graphics-context setup failed — do not call `Renderer::init`.
         * @details Use this guard in `main` before `Application::run()`.
         */
        bool isValid() const;

        // pollevents is for input handling because it updates the state of the keyboard and mouse buttons.
        // swapbuffers is for rendering because it displays the rendered frame on the screen.
        void pollEvents();
        void swapBuffers();

        void setCursorMode(CursorMode mode);
        void setRawMouseMotion(bool enabled);
        graphicsContext &getGraphicsContext();
        const graphicsContext &getGraphicsContext() const;

    private:
        friend void *detail::nativeWindowHandleForInput(const Window &window);

        std::unique_ptr<WindowImpl> m_impl;
    };
}
