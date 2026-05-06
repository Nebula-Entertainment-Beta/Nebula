/**
 * @file input.h
 * @brief GLFW-backed **keyboard, mouse buttons, cursor delta, and scroll** with per-frame edge detection.
 *
 * **Owns:** Per-key and per-button held / pressed-this-frame / released-this-frame arrays; mouse deltas reset in `beginFrame`.
 *
 * Gamepad is **not** implemented here yet. For gameplay-level keys, prefer `ActionMapping` on top of this type.
 */
#pragma once
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <array>
#include <cstdint>


namespace Nebula{
  class Input{
    public:
      Input() = default;

      void attachToWindow(GLFWwindow* window);
      void detachFromWindow();
      void beginFrame();

      bool isKeyDown(int key) const;
      bool wasKeyPressed(int key) const;
      bool wasKeyReleased(int key) const;
      bool isMouseButtonDown(int MouseButton) const;
      bool wasMouseButtonPressed(int MouseButton) const;
      bool wasMouseButtonReleased(int MouseButton) const;

      
      float mouseDeltaX() const{return m_mouseDeltaX;}

      float mouseDeltaY() const{return m_mouseDeltaY; }
      float mouseScrollDeltaY() const{return m_mouseScrollDeltaY;}
    
    private:
      static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
      static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
      static void cursorPosCallback(GLFWwindow* window, double x, double y);
      static void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);

      void setWindowUserPointer(GLFWwindow* window, Input* input){
        glfwSetWindowUserPointer(window, input);
      }

      GLFWwindow* m_window = nullptr;

      std::array<bool, GLFW_KEY_LAST + 1> m_keyDown{};
      std::array<bool, GLFW_KEY_LAST + 1> m_keyPressed{};
      std::array<bool, GLFW_KEY_LAST + 1> m_keyReleased{};

      std::array<bool, GLFW_MOUSE_BUTTON_LAST + 1> m_mouseButtonDown{};
      std::array<bool, GLFW_MOUSE_BUTTON_LAST + 1> m_mouseButtonPressed{};
      std::array<bool, GLFW_MOUSE_BUTTON_LAST + 1> m_mouseButtonReleased{};

      float m_mouseDeltaX = 0.0f;
      float m_mouseDeltaY = 0.0f;
      float m_mouseScrollDeltaY = 0.0f;

      double m_lastCursorX = 0.0;
      double m_lastCursorY = 0.0;

      bool m_haveCursorSample = false;


  };

}
