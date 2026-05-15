/**
 * @file input.h
 * @brief GLFW-backed **keyboard, mouse buttons, cursor delta, and scroll** with per-frame edge detection.
 *
 * **Owns:** Per-key and per-button held / pressed-this-frame / released-this-frame arrays; mouse deltas reset in `beginFrame`.
 *
 * Gamepad is **not** implemented here yet. For gameplay-level keys, prefer `ActionMapping` on top of this type.
 *
 * GLFW is not included here so games only need Nebula's public `include/` directory.
 * Slot counts match GLFW 3.x (`GLFW_KEY_LAST` = 348, `GLFW_MOUSE_BUTTON_LAST` = 7).
 */
#pragma once
#include <array>
#include <cstdint>

struct GLFWwindow;

namespace Nebula
{
  class Window;

  inline constexpr int kInputKeySlotCount = 349;         // GLFW_KEY_LAST + 1
  inline constexpr int kInputMouseButtonSlotCount = 8;   // GLFW_MOUSE_BUTTON_LAST + 1

  class Input
  {
  public:
    Input() = default;

    void attachToWindow(Window &window);
    void detachFromWindow();
    void beginFrame();

    bool isKeyDown(int key) const;
    bool wasKeyPressed(int key) const;
    bool wasKeyReleased(int key) const;
    bool isMouseButtonDown(int MouseButton) const;
    bool wasMouseButtonPressed(int MouseButton) const;
    bool wasMouseButtonReleased(int MouseButton) const;

    float mouseDeltaX() const { return m_mouseDeltaX; }

    float mouseDeltaY() const { return m_mouseDeltaY; }
    float mouseScrollDeltaY() const { return m_mouseScrollDeltaY; }

  private:
    static void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods);
    static void mouseButtonCallback(GLFWwindow *window, int button, int action, int mods);
    static void cursorPosCallback(GLFWwindow *window, double x, double y);
    static void scrollCallback(GLFWwindow *window, double xoffset, double yoffset);

    GLFWwindow *m_window = nullptr;

    std::array<bool, kInputKeySlotCount> m_keyDown{};
    std::array<bool, kInputKeySlotCount> m_keyPressed{};
    std::array<bool, kInputKeySlotCount> m_keyReleased{};

    std::array<bool, kInputMouseButtonSlotCount> m_mouseButtonDown{};
    std::array<bool, kInputMouseButtonSlotCount> m_mouseButtonPressed{};
    std::array<bool, kInputMouseButtonSlotCount> m_mouseButtonReleased{};

    float m_mouseDeltaX = 0.0f;
    float m_mouseDeltaY = 0.0f;
    float m_mouseScrollDeltaY = 0.0f;

    double m_lastCursorX = 0.0;
    double m_lastCursorY = 0.0;

    bool m_haveCursorSample = false;
  };

} // namespace Nebula
