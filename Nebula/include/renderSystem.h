#pragma once
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
  };

  void renderScene(const RenderSystemContext &ctx);
}