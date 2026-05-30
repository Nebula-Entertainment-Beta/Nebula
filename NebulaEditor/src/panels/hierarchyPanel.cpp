
#include "hierarchyPanel.h"
#include <imgui.h>
#include <imgui_internal.h>

namespace Editor
{

  void HierarchyPanel::drawHierarchyPanel(Nebula::Scene &scene, EditorState &state)
  {

    ImGui::Begin("Hierarchy");
    if (ImGui::Button("Create Entity"))
    {
      state.selectedEntity = scene.createEntity();
      state.sceneDirty = true;
    }
    for (Nebula::Entity entity : scene.getAllEntities())
    {
      std::string label = "Entity " + std::to_string(entity.id);
      if (scene.hasComponent<Nebula::TagComponent>(entity))
      {
        label = scene.getComponent<Nebula::TagComponent>(entity).tag;
      }
      ImGui::Selectable(label.c_str(), state.selectedEntity == entity);
      if (ImGui::IsItemClicked())
      {
        state.selectedEntity = entity;
      }
    }
    if (ImGui::Button("Delete Entity"))
    {
      if (state.selectedEntity != Nebula::Entity())
      {
        scene.destroyEntity(state.selectedEntity);
        state.selectedEntity = Nebula::Entity();
        state.sceneDirty = true;
      }
    }
    ImGui::End();
  }
}