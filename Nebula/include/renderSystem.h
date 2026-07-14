#pragma once
#include <cstdint>
#include "math_types.h"

namespace Nebula
{
  class Scene;
  class AssetManager;
  class Renderer;
  class Window;
  class Camera3D;

  struct RenderSystemContext
  {
    Scene &scene;
    AssetManager &assets;
    Renderer &renderer;
    Window &window;
    int viewportX = 0, viewportY = 0;
    uint32_t viewportWidth = 0, viewportHeight = 0;
    /** When set, Scene View / editor rendering uses this camera instead of the scene primary camera. */
    const Camera3D *overrideCamera = nullptr;
    /** When set, takes precedence over @p overrideCamera (used by editor fly camera). */
    const Mat4 *overrideViewProjection = nullptr;
  };

  void renderScene(const RenderSystemContext &ctx);
}