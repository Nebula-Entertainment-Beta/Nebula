/**
 * @file input.cpp
 * @brief GLFW callbacks (`Window` user pointer → `Input`) and query methods for keys/mouse.
 */
#include "input.h"
#include "Window.h"
#include <GLFW/glfw3.h>
#include <algorithm>


namespace Nebula{
  void Input::attachToWindow(Window& window)
  {
      GLFWwindow* const w = window.m_window;
      if (!w) {
          return;
      }
      m_window = w;
      glfwSetKeyCallback(m_window, keyCallback);
      glfwSetMouseButtonCallback(m_window, mouseButtonCallback);
      glfwSetCursorPosCallback(m_window, cursorPosCallback);
      glfwSetScrollCallback(m_window, scrollCallback);
      glfwSetWindowUserPointer(m_window, this);
  }

  void Input::detachFromWindow()
  {
      if (m_window) {
          glfwSetKeyCallback(m_window, nullptr);
          glfwSetMouseButtonCallback(m_window, nullptr);
          glfwSetCursorPosCallback(m_window, nullptr);
          glfwSetScrollCallback(m_window, nullptr);
          glfwSetWindowUserPointer(m_window, nullptr);
          m_window = nullptr;
      }
  }

  void Input::beginFrame(){
      std::fill(m_keyPressed.begin(), m_keyPressed.end(), false);
      std::fill(m_keyReleased.begin(), m_keyReleased.end(), false);
      std::fill(m_mouseButtonPressed.begin(), m_mouseButtonPressed.end(), false);
      std::fill(m_mouseButtonReleased.begin(), m_mouseButtonReleased.end(), false);
      m_mouseDeltaX = 0.0f;
      m_mouseDeltaY = 0.0f;
      m_mouseScrollDeltaY = 0.0f;



  }

  

  //query function definitions and callback implementations 
  void Input::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
  {
      Input* input = static_cast<Input*>(glfwGetWindowUserPointer(window));
      if (!input) return;

      if (key < 0 || key > GLFW_KEY_LAST) return;

      if (action == GLFW_PRESS) {
          input->m_keyDown[key] = true;
          input->m_keyPressed[key] = true;
      } else if (action == GLFW_RELEASE) {
          input->m_keyDown[key] = false;
          input->m_keyReleased[key] = true;
      }
  }

  void Input::mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
  {
      Input* input = static_cast<Input*>(glfwGetWindowUserPointer(window));
      if (!input) return;

      if (button < 0 || button > GLFW_MOUSE_BUTTON_LAST) return;

      if (action == GLFW_PRESS) {
          input->m_mouseButtonDown[button] = true;
          input->m_mouseButtonPressed[button] = true;
      } else if (action == GLFW_RELEASE) {
          input->m_mouseButtonDown[button] = false;
          input->m_mouseButtonReleased[button] = true;
      }
  }

  void Input::cursorPosCallback(GLFWwindow* window, double x, double y)
  {
      Input* input = static_cast<Input*>(glfwGetWindowUserPointer(window));
      if (!input) return;

      if (input->m_haveCursorSample) {
          input->m_mouseDeltaX += static_cast<float>(x - input->m_lastCursorX);
          input->m_mouseDeltaY += static_cast<float>(y - input->m_lastCursorY);
      } else {
          input->m_haveCursorSample = true;
      }

      input->m_lastCursorX = x;
      input->m_lastCursorY = y;
  }

  void Input::scrollCallback(GLFWwindow* window, double xoffset, double yoffset)
  {
      Input* input = static_cast<Input*>(glfwGetWindowUserPointer(window));
      if (!input) return;

      input->m_mouseScrollDeltaY += static_cast<float>(yoffset);
  }

  // Query functions
  bool Input::isKeyDown(int key) const
  {
      if (key < 0 || key > GLFW_KEY_LAST) return false;
      return m_keyDown[key];
  }

  bool Input::wasKeyPressed(int key) const
  {
      if (key < 0 || key > GLFW_KEY_LAST) return false;
      return m_keyPressed[key];
  }

  bool Input::wasKeyReleased(int key) const
  {
      if (key < 0 || key > GLFW_KEY_LAST) return false;
      return m_keyReleased[key];
  }

  bool Input::isMouseButtonDown(int button) const
  {
      if (button < 0 || button > GLFW_MOUSE_BUTTON_LAST) return false;
      return m_mouseButtonDown[button];
  }


  bool Input::wasMouseButtonPressed(int button) const
  {
      if (button < 0 || button > GLFW_MOUSE_BUTTON_LAST) return false;
      return m_mouseButtonPressed[button];
  }

  bool Input::wasMouseButtonReleased(int button) const
  {
      if (button < 0 || button > GLFW_MOUSE_BUTTON_LAST) return false;
      return m_mouseButtonReleased[button];
  }




}
