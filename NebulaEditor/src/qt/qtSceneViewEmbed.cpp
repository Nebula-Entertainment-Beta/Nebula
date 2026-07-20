#include "qt/qtSceneViewEmbed.h"

#include "Window.h"

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#define GLFW_EXPOSE_NATIVE_WIN32
#endif

#include <GLFW/glfw3.h>

#ifdef _WIN32
#include <GLFW/glfw3native.h>
#endif

namespace Editor
{
  WId sceneViewNativeHandle(Nebula::Window &window)
  {
    auto *glfwWindow = static_cast<GLFWwindow *>(Nebula::detail::nativeWindowHandleForInput(window));
    if (glfwWindow == nullptr)
    {
      return 0;
    }
    // Borderless child look: Qt owns the frame once embedded.
    glfwSetWindowAttrib(glfwWindow, GLFW_DECORATED, GLFW_FALSE);
#ifdef _WIN32
    return reinterpret_cast<WId>(glfwGetWin32Window(glfwWindow));
#else
    return 0;
#endif
  }
}
