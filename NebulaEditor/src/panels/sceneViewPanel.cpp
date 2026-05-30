#include "sceneViewPanel.h"
#include "renderSystem.h"

#include <imgui.h>

namespace Editor
{

  void SceneViewPanel::drawSceneViewPanel(EditorState &state, SceneViewFrameBuffer &framebuffer, Nebula::Scene &scene,
                                          Nebula::AssetManager &assets, Nebula::Renderer &renderer, Nebula::Window &window)
  {
    (void)state;
    ImGui::Begin("Scene View");
    ImVec2 size = ImGui::GetContentRegionAvail();
    if (size.x > 0 && size.y > 0)
    {
      framebuffer.resize((int)size.x, (int)size.y);
      framebuffer.bind();
      framebuffer.clear();
      Nebula::renderScene({scene, assets, renderer, window, 0, 0, (uint32_t)size.x, (uint32_t)size.y});
      framebuffer.unbind();
      ImGui::Image((ImTextureID)(intptr_t)framebuffer.colorTextureId(), size, ImVec2(0, 1), ImVec2(1, 0));
    }
    ImGui::End();
  }
}
