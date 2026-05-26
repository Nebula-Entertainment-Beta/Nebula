#pragma once
#include "editorState.h"
#include "Nebula.h"
#include "render/sceneViewFrameBuffer.h"

namespace Editor
{

  class SceneViewPanel
  {
  public:
    void drawSceneViewPanel(EditorState &state, SceneViewFrameBuffer &framebuffer,
                            Nebula::Scene &scene, Nebula::AssetManager &assets, Nebula::Renderer &renderer, Nebula::Window &window);

  private:
  };
}