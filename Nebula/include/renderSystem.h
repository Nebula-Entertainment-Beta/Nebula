#pragma once
#include <cstdint>
#include "math_types.h"

namespace Nebula
{
  class Scene;
  class AssetManager;
  class Renderer;
  class Window;

  struct RenderSystemContext
  {
    Scene &scene;
    AssetManager &assets;
    Renderer &renderer;
    Window &window;
    int viewportX = 0, viewportY = 0;
    uint32_t viewportWidth = 0, viewportHeight = 0;
  };

  void renderScene(const RenderSystemContext &ctx);
}