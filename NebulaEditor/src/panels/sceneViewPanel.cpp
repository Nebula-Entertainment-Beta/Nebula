#include "sceneViewPanel.h"
#include "collider_debug.h"
#include "renderSystem.h"

#include <imgui.h>

namespace Editor
{

  void SceneViewPanel::drawSceneViewPanel(EditorState &state, SceneViewFrameBuffer &framebuffer, Nebula::Scene &scene,
                                          Nebula::AssetManager &assets, Nebula::Renderer &renderer, Nebula::Window &window)
  {
    ImGui::Begin("Scene View");
    ImGui::Checkbox("Show Colliders", &state.showColliderGizmos);
    ImVec2 size = ImGui::GetContentRegionAvail();
    if (size.x > 0 && size.y > 0)
    {
      framebuffer.resize((int)size.x, (int)size.y);
      framebuffer.bind();
      framebuffer.clear();
      Nebula::RenderSystemContext renderCtx{scene, assets, renderer, window, 0, 0, (uint32_t)size.x, (uint32_t)size.y};
      Nebula::renderScene(renderCtx);
      if (state.showColliderGizmos)
      {
        Nebula::renderColliderGizmos(renderCtx);
      }
      framebuffer.unbind();
      ImGui::Image((ImTextureID)(intptr_t)framebuffer.colorTextureId(), size, ImVec2(0, 1), ImVec2(1, 0));
    }
    ImGui::End();
  }
}
