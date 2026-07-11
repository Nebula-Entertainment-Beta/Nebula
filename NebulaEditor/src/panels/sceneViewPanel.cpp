#include "sceneViewPanel.h"
#include "collider_debug.h"
#include "transform_gizmo.h"
#include "renderSystem.h"
#include "component.h"

#include <imgui.h>

namespace Editor
{

  void SceneViewPanel::drawSceneViewPanel(EditorState &state, SceneViewFrameBuffer &framebuffer, Nebula::Scene &scene,
                                          Nebula::AssetManager &assets, Nebula::Renderer &renderer, Nebula::Window &window)
  {
    ImGui::Begin("Scene View");
    ImGui::Checkbox("Show Colliders", &state.showColliderGizmos);
    ImGui::SameLine();
    ImGui::Checkbox("Show Transform Gizmo", &state.showTransformGizmo);
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
      if (state.showTransformGizmo && scene.isValidEntity(state.selectedEntity))
      {
        Nebula::renderTransformGizmo(renderCtx, state.selectedEntity);
      }
      framebuffer.unbind();
      ImGui::Image((ImTextureID)(intptr_t)framebuffer.colorTextureId(), size, ImVec2(0, 1), ImVec2(1, 0));

      if (state.showTransformGizmo && scene.isValidEntity(state.selectedEntity) &&
          scene.hasComponent<Nebula::TransformComponent>(state.selectedEntity) && ImGui::IsItemHovered() &&
          ImGui::IsMouseDragging(ImGuiMouseButton_Left))
      {
        auto &transform = scene.getComponent<Nebula::TransformComponent>(state.selectedEntity).transform;
        const Nebula::Vec3 pos = transform.getPosition();
        const ImVec2 delta = ImGui::GetIO().MouseDelta;
        transform.setPosition({pos.x + delta.x * 0.02f, pos.y, pos.z - delta.y * 0.02f});
        state.sceneDirty = true;
      }
    }
    ImGui::End();
  }
}
