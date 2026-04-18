/**
 * @file Window.h
 * @brief Creates an OS window and an OpenGL context using GLFW.
 *
 * **Big picture:** Your game needs a drawable surface. GLFW talks to the operating system to open
 * a window; GLAD loads OpenGL function pointers after a context exists. Only call rendering code
 * if `isValid()` is true — otherwise setup failed (no GPU context).
 */
#pragma once
#include <string_view>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

namespace Nebula
{
    /**
     * @brief Owns a `GLFWwindow` and the OpenGL context created for it.
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

        /** Swaps the front/back buffer and processes input events (call once per frame). */
        void update();

        /** @return True when the user closed the window or the window failed. */
        bool shouldWindowClose();

        ~Window();

        /**
         * @brief False if GLFW/GLAD setup failed — do not call `Renderer` or other GL APIs.
         * @details Use this guard in `main` before `Renderer::init()`.
         */
        bool isValid() const { return m_window != nullptr && m_glReady; }

        /**
         * @brief Pixel size of the drawable framebuffer (may differ from window size on Hi-DPI).
         * Use this for `glViewport` and projection aspect, not the constructor width/height alone.
         */
        void getFramebufferSize(int& outWidth, int& outHeight) const;

        /**
         * @brief True while key is held (`GLFW_KEY_*` from glfw3.h). Until `Input` exists, use this from `onUpdate`.
         */
        bool isKeyHeld(int glfwKey) const;

    private:
        GLFWwindow* m_window = nullptr;
        bool m_glfwInitialized = false;
        bool m_glReady = false;
    };
}
