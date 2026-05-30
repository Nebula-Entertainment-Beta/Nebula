
#include "hierarchyPanel.h"
#include <imgui.h>

namespace Editor
{

  void HierarchyPanel::setEntityActions(std::function<void()> createEntity, std::function<void()> deleteEntity)
  {
    m_createEntity = std::move(createEntity);
    m_deleteEntity = std::move(deleteEntity);
  }

  void HierarchyPanel::drawHierarchyPanel(Nebula::Scene &scene, EditorState &state)
  {
    ImGui::Begin("Hierarchy");

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

    if (ImGui::BeginPopupContextWindow("HierarchyContext"))
    {
      if (m_createEntity && ImGui::MenuItem("Create Entity"))
      {
        m_createEntity();
      }
      const bool canDelete =
          state.selectedEntity != Nebula::Entity() && scene.isValidEntity(state.selectedEntity);
      if (!canDelete)
      {
        ImGui::BeginDisabled();
      }
      if (m_deleteEntity && ImGui::MenuItem("Delete Entity"))
      {
        m_deleteEntity();
      }
      if (!canDelete)
      {
        ImGui::EndDisabled();
      }
      ImGui::EndPopup();
    }

    ImGui::End();
  }
}
