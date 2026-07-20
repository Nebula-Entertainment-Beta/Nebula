/**
 * @file input.h
 * @brief GLFW-backed **keyboard, mouse buttons, cursor delta, and scroll** with per-frame edge detection.
 *
 * **Owns:** Per-key and per-button held / pressed-this-frame / released-this-frame arrays; mouse
 * deltas reset in `beginFrame`. Call `beginFrame` at the **end** of each frame (after systems read
 * input). Under a foreign event loop (Qt), GLFW callbacks can fire between frames; clearing at the
 * start of the next frame would drop those edges.
 *
 * Gamepad is **not** implemented here yet. For gameplay-level keys, prefer `ActionMapping` on top of this type.
 *
 * No GLFW types appear in this header — games only need Nebula's public `include/` directory.
 * Keyboard layout uses `Tasto` / `TastoDelMouse` from `inputTypes.h` (dense indices `0 .. Count-1`).
 */
#pragma once

#include "inputTypes.h"

#include <array>
#include <cstdint>

namespace Nebula
{
  class Window;

  namespace InputDetail
  {
    void onKey(void *windowHandle, int key, int scancode, int action, int mods);
    void onMouseButton(void *windowHandle, int button, int action, int mods);
    void onCursorPos(void *windowHandle, double x, double y);
    void onScroll(void *windowHandle, double xoffset, double yoffset);
  }

  class Input
  {
    friend void InputDetail::onKey(void *windowHandle, int key, int scancode, int action, int mods);
    friend void InputDetail::onMouseButton(void *windowHandle, int button, int action, int mods);
    friend void InputDetail::onCursorPos(void *windowHandle, double x, double y);
    friend void InputDetail::onScroll(void *windowHandle, double xoffset, double yoffset);

  public:
    Input() = default;

    void attach(Window &window);
    void detach();
    void beginFrame();

    bool isKeyDown(Tasto key) const;
    bool wasKeyPressed(Tasto key) const;
    bool wasKeyReleased(Tasto key) const;
    bool isMouseButtonDown(TastoDelMouse button) const;
    bool wasMouseButtonPressed(TastoDelMouse button) const;
    bool wasMouseButtonReleased(TastoDelMouse button) const;

    float mouseDeltaX() const { return m_mouseDeltaX; }

    float mouseDeltaY() const { return m_mouseDeltaY; }
    float mouseScrollDeltaY() const { return m_mouseScrollDeltaY; }
    /** Cursor position in window coordinates (GLFW content area). */
    float cursorX() const { return static_cast<float>(m_lastCursorX); }
    float cursorY() const { return static_cast<float>(m_lastCursorY); }
    bool hasCursorSample() const { return m_haveCursorSample; }

  private:
    void *m_nativeWindow = nullptr;

    std::array<bool, tastoSlotCount()> m_keyDown{};
    std::array<bool, tastoSlotCount()> m_keyPressed{};
    std::array<bool, tastoSlotCount()> m_keyReleased{};

    std::array<bool, tastoDelMouseSlotCount()> m_mouseButtonDown{};
    std::array<bool, tastoDelMouseSlotCount()> m_mouseButtonPressed{};
    std::array<bool, tastoDelMouseSlotCount()> m_mouseButtonReleased{};

    float m_mouseDeltaX = 0.0f;
    float m_mouseDeltaY = 0.0f;
    float m_mouseScrollDeltaY = 0.0f;

    double m_lastCursorX = 0.0;
    double m_lastCursorY = 0.0;

    bool m_haveCursorSample = false;
  };

} // namespace Nebula
