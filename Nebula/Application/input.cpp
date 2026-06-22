/**
 * @file input.cpp
 * @brief GLFW callbacks (`Window` user pointer → `Input`) and query methods for keys/mouse.
 */
#include "input.h"
#include "Window.h"
#include "detail/window_native.h"
#include "inputTypes.h"

#include <GLFW/glfw3.h>
#include <algorithm>
#include <optional>

namespace Nebula
{
    namespace
    {

        std::optional<Tasto> glfwKeyToTasto(int glfwKey)
        {
            switch (glfwKey)
            {
            case GLFW_KEY_1:
                return Tasto::num1;
            case GLFW_KEY_SPACE:
                return Tasto::space;
            case GLFW_KEY_2:
                return Tasto::num2;
            case GLFW_KEY_3:
                return Tasto::num3;
            case GLFW_KEY_4:
                return Tasto::num4;
            case GLFW_KEY_5:
                return Tasto::num5;
            case GLFW_KEY_6:
                return Tasto::num6;
            case GLFW_KEY_7:
                return Tasto::num7;
            case GLFW_KEY_8:
                return Tasto::num8;
            case GLFW_KEY_9:
                return Tasto::num9;
            case GLFW_KEY_0:
                return Tasto::num0;
            case GLFW_KEY_TAB:
                return Tasto::tab;
            case GLFW_KEY_Q:
                return Tasto::q;
            case GLFW_KEY_W:
                return Tasto::w;
            case GLFW_KEY_E:
                return Tasto::e;
            case GLFW_KEY_R:
                return Tasto::r;
            case GLFW_KEY_T:
                return Tasto::t;
            case GLFW_KEY_Y:
                return Tasto::y;
            case GLFW_KEY_U:
                return Tasto::u;
            case GLFW_KEY_I:
                return Tasto::i;
            case GLFW_KEY_O:
                return Tasto::o;
            case GLFW_KEY_P:
                return Tasto::p;
            case GLFW_KEY_A:
                return Tasto::a;
            case GLFW_KEY_S:
                return Tasto::s;
            case GLFW_KEY_D:
                return Tasto::d;
            case GLFW_KEY_F:
                return Tasto::f;
            case GLFW_KEY_G:
                return Tasto::g;
            case GLFW_KEY_H:
                return Tasto::h;
            case GLFW_KEY_J:
                return Tasto::j;
            case GLFW_KEY_K:
                return Tasto::k;
            case GLFW_KEY_L:
                return Tasto::l;
            case GLFW_KEY_Z:
                return Tasto::z;
            case GLFW_KEY_X:
                return Tasto::x;
            case GLFW_KEY_C:
                return Tasto::c;
            case GLFW_KEY_V:
                return Tasto::v;
            case GLFW_KEY_B:
                return Tasto::b;
            case GLFW_KEY_N:
                return Tasto::n;
            case GLFW_KEY_M:
                return Tasto::m;
            default:
                return std::nullopt;
            }
        }

        std::optional<TastoDelMouse> glfwMouseButtonToTastoDelMouse(int glfwButton)
        {
            switch (glfwButton)
            {
            case GLFW_MOUSE_BUTTON_LEFT:
                return TastoDelMouse::left;
            case GLFW_MOUSE_BUTTON_RIGHT:
                return TastoDelMouse::right;
            case GLFW_MOUSE_BUTTON_MIDDLE:
                return TastoDelMouse::middle;
            case GLFW_MOUSE_BUTTON_4:
                return TastoDelMouse::tastoMouse1;
            case GLFW_MOUSE_BUTTON_5:
                return TastoDelMouse::tastoMouse2;
            default:
                return std::nullopt;
            }
        }

        void glfwKeyBridge(GLFWwindow *window, int key, int scancode, int action, int mods)
        {
            InputDetail::onKey(static_cast<void *>(window), key, scancode, action, mods);
        }

        void glfwMouseButtonBridge(GLFWwindow *window, int button, int action, int mods)
        {
            InputDetail::onMouseButton(static_cast<void *>(window), button, action, mods);
        }

        void glfwCursorPosBridge(GLFWwindow *window, double x, double y)
        {
            InputDetail::onCursorPos(static_cast<void *>(window), x, y);
        }

        void glfwScrollBridge(GLFWwindow *window, double xoffset, double yoffset)
        {
            InputDetail::onScroll(static_cast<void *>(window), xoffset, yoffset);
        }
    } // namespace

    void InputDetail::onKey(void *windowHandle, int key, int scancode, int action, int mods)
    {
        (void)scancode;
        (void)mods;
        auto *window = static_cast<GLFWwindow *>(windowHandle);
        Input *input = static_cast<Input *>(glfwGetWindowUserPointer(window));
        if (!input)
            return;

        const std::optional<Tasto> tasto = glfwKeyToTasto(key);
        if (!tasto.has_value())
            return;

        const std::size_t index = static_cast<std::size_t>(*tasto);
        if (*tasto == Tasto::Unknown || *tasto == Tasto::Count || index >= tastoSlotCount())
            return;

        if (action == GLFW_PRESS)
        {
            input->m_keyDown[index] = true;
            input->m_keyPressed[index] = true;
        }
        else if (action == GLFW_RELEASE)
        {
            input->m_keyDown[index] = false;
            input->m_keyReleased[index] = true;
        }
    }

    void InputDetail::onMouseButton(void *windowHandle, int button, int action, int mods)
    {
        (void)mods;
        auto *window = static_cast<GLFWwindow *>(windowHandle);
        Input *input = static_cast<Input *>(glfwGetWindowUserPointer(window));
        if (!input)
            return;

        const std::optional<TastoDelMouse> mb = glfwMouseButtonToTastoDelMouse(button);
        if (!mb.has_value())
            return;

        const std::size_t index = static_cast<std::size_t>(*mb);
        if (index >= tastoDelMouseSlotCount())
            return;

        if (action == GLFW_PRESS)
        {
            input->m_mouseButtonDown[index] = true;
            input->m_mouseButtonPressed[index] = true;
        }
        else if (action == GLFW_RELEASE)
        {
            input->m_mouseButtonDown[index] = false;
            input->m_mouseButtonReleased[index] = true;
        }
    }

    void InputDetail::onCursorPos(void *windowHandle, double x, double y)
    {
        auto *window = static_cast<GLFWwindow *>(windowHandle);
        Input *input = static_cast<Input *>(glfwGetWindowUserPointer(window));
        if (!input)
            return;

        if (input->m_haveCursorSample)
        {
            input->m_mouseDeltaX += static_cast<float>(x - input->m_lastCursorX);
            input->m_mouseDeltaY += static_cast<float>(y - input->m_lastCursorY);
        }
        else
        {
            input->m_haveCursorSample = true;
        }

        input->m_lastCursorX = x;
        input->m_lastCursorY = y;
    }

    void InputDetail::onScroll(void *windowHandle, double xoffset, double yoffset)
    {
        (void)xoffset;
        auto *window = static_cast<GLFWwindow *>(windowHandle);
        Input *input = static_cast<Input *>(glfwGetWindowUserPointer(window));
        if (!input)
            return;

        input->m_mouseScrollDeltaY += static_cast<float>(yoffset);
    }

    void Input::attach(Window &window)
    {
        void *const h = detail::nativeWindowHandleForInput(window);
        if (!h)
            return;
        auto *const w = static_cast<GLFWwindow *>(h);
        m_nativeWindow = h;
        glfwSetKeyCallback(w, glfwKeyBridge);
        glfwSetMouseButtonCallback(w, glfwMouseButtonBridge);
        glfwSetCursorPosCallback(w, glfwCursorPosBridge);
        glfwSetScrollCallback(w, glfwScrollBridge);
        glfwSetWindowUserPointer(w, this);
    }

    void Input::detach()
    {
        if (m_nativeWindow)
        {
            auto *w = static_cast<GLFWwindow *>(m_nativeWindow);
            glfwSetKeyCallback(w, nullptr);
            glfwSetMouseButtonCallback(w, nullptr);
            glfwSetCursorPosCallback(w, nullptr);
            glfwSetScrollCallback(w, nullptr);
            glfwSetWindowUserPointer(w, nullptr);
            m_nativeWindow = nullptr;
        }
    }

    void Input::beginFrame()
    {
        std::fill(m_keyPressed.begin(), m_keyPressed.end(), false);
        std::fill(m_keyReleased.begin(), m_keyReleased.end(), false);
        std::fill(m_mouseButtonPressed.begin(), m_mouseButtonPressed.end(), false);
        std::fill(m_mouseButtonReleased.begin(), m_mouseButtonReleased.end(), false);
        m_mouseDeltaX = 0.0f;
        m_mouseDeltaY = 0.0f;
        m_mouseScrollDeltaY = 0.0f;
    }

    bool Input::isKeyDown(Tasto key) const
    {
        if (key == Tasto::Unknown || key == Tasto::Count)
            return false;
        const std::size_t i = static_cast<std::size_t>(key);
        if (i >= tastoSlotCount())
            return false;
        return m_keyDown[i];
    }

    bool Input::wasKeyPressed(Tasto key) const
    {
        if (key == Tasto::Unknown || key == Tasto::Count)
            return false;
        const std::size_t i = static_cast<std::size_t>(key);
        if (i >= tastoSlotCount())
            return false;
        return m_keyPressed[i];
    }

    bool Input::wasKeyReleased(Tasto key) const
    {
        if (key == Tasto::Unknown || key == Tasto::Count)
            return false;
        const std::size_t i = static_cast<std::size_t>(key);
        if (i >= tastoSlotCount())
            return false;
        return m_keyReleased[i];
    }

    bool Input::isMouseButtonDown(TastoDelMouse button) const
    {
        if (button == TastoDelMouse::Count)
            return false;
        const std::size_t i = static_cast<std::size_t>(button);
        if (i >= tastoDelMouseSlotCount())
            return false;
        return m_mouseButtonDown[i];
    }

    bool Input::wasMouseButtonPressed(TastoDelMouse button) const
    {
        if (button == TastoDelMouse::Count)
            return false;
        const std::size_t i = static_cast<std::size_t>(button);
        if (i >= tastoDelMouseSlotCount())
            return false;
        return m_mouseButtonPressed[i];
    }

    bool Input::wasMouseButtonReleased(TastoDelMouse button) const
    {
        if (button == TastoDelMouse::Count)
            return false;
        const std::size_t i = static_cast<std::size_t>(button);
        if (i >= tastoDelMouseSlotCount())
            return false;
        return m_mouseButtonReleased[i];
    }

} // namespace Nebula
