/**
 * @file Window.h
 * @brief Creates an OS window and an OpenGL context using GLFW.
 *
 * **Big picture:** Your game needs a drawable surface. GLFW talks to the operating system to open
 * a window; GLAD loads OpenGL function pointers after a context exists. Only call rendering code
 * if `isValid()` is true — otherwise setup failed (no GPU context).
 */
#pragma once
#include <memory>
#include <string_view>

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

    /**
     * @brief Owns a native window and an OpenGL context (GLFW + GLAD in the implementation).
     */
    class Window
    {
    public:
        /**
         * @brief Opens a window, creates the GL context, and loads GLAD.
         * @param title  Window title bar text.
         * @param width  Initial width in pixels.
         * @param height Initial height in pixels.
         */
        Window(std::string_view title, int width, int height);

        /** @return True when the user closed the window or the window failed. */
        bool shouldWindowClose();

        ~Window();

        /**
         * @brief Pixel size of the drawable framebuffer (may differ from window size on Hi-DPI).
         * Use this for `glViewport` and projection aspect, not the constructor width/height alone.
         */
        void getFramebufferSize(int &outWidth, int &outHeight) const;

        /**
         * @brief False if GLFW/GLAD setup failed — do not call `Renderer` or other GL APIs.
         * @details Use this guard in `main` before `Renderer::init()`.
         */
        bool isValid() const;

        // pollevents is for input handling because it updates the state of the keyboard and mouse buttons.
        // swapbuffers is for rendering because it displays the rendered frame on the screen.
        void pollEvents();
        void swapBuffers();

        void setCursorMode(CursorMode mode);
        void setRawMouseMotion(bool enabled);

    private:
        friend void *detail::nativeWindowHandleForInput(const Window &window);

        std::unique_ptr<WindowImpl> m_impl;
    };
}
