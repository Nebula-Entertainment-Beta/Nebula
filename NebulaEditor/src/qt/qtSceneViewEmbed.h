#pragma once

#include <qwindowdefs.h>

namespace Nebula
{
  class Window;
}

namespace Editor
{
  /** Native OS handle of the engine's GLFW window, for embedding as a Qt child (0 if unavailable). */
  WId sceneViewNativeHandle(Nebula::Window &window);
}
