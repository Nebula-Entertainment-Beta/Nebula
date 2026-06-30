
#include "hierarchyPanel.h"
#include "prefabInstance.h"
#include <imgui.h>

namespace Editor
{
  namespace
  {
    constexpr const char *kKnownPrefabs[] = {
        "prefabs/enemy.prefab",
        "prefabs/platform.prefab",
        "prefabs/bounce_pad.prefab",
        "prefabs/wind_volume.prefab",
    };
  }

  void HierarchyPanel::setEntityActions(std::function<void(const char *templateId)> factory, std::function<void()> deleteEntity)
  {
    m_entityFactory = std::move(factory);
    m_deleteEntity = std::move(deleteEntity);
  }

  void HierarchyPanel::setPrefabActions(std::function<void(std::string_view prefabPath)> instantiatePrefab,
                                        std::function<void()> saveSelectedAsPrefab)
  {
    m_instantiatePrefab = std::move(instantiatePrefab);
    m_saveSelectedAsPrefab = std::move(saveSelectedAsPrefab);
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
      if (scene.hasComponent<Nebula::PrefabInstanceComponent>(entity))
      {
        label += " [" + scene.getComponent<Nebula::PrefabInstanceComponent>(entity).prefabPath + "]";
      }

      ImGui::PushID(static_cast<int>(entity.id));
      ImGui::Selectable(label.c_str(), state.selectedEntity == entity);
      if (ImGui::IsItemClicked())
      {
        state.selectedEntity = entity;
      }

      const bool isSelected = state.selectedEntity == entity;
      if (ImGui::BeginPopupContextItem())
      {
        if (isSelected && m_saveSelectedAsPrefab && ImGui::MenuItem("Save as Prefab"))
        {
          m_saveSelectedAsPrefab();
        }
        if (m_deleteEntity && ImGui::MenuItem("Delete Entity"))
        {
          m_deleteEntity();
        }
        ImGui::EndPopup();
      }
      ImGui::PopID();
    }

    if (ImGui::BeginPopupContextWindow("HierarchyContext"))
    {
      if (m_entityFactory)
      {
        if (ImGui::MenuItem("Create Empty"))
        {
          m_entityFactory("empty");
        }
        if (ImGui::MenuItem("Create Mesh Cube"))
        {
          m_entityFactory("cube");
        }
        if (ImGui::MenuItem("Create Enemy Placeholder"))
        {
          m_entityFactory("enemy");
        }
        if (ImGui::MenuItem("Create Platform"))
        {
          m_entityFactory("platform");
        }
        if (ImGui::MenuItem("Create Bounce Pad"))
        {
          m_entityFactory("bouncePad");
        }
        if (ImGui::MenuItem("Create Wind Volume"))
        {
          m_entityFactory("windVolume");
        }
      }
      if (m_instantiatePrefab && ImGui::BeginMenu("Instantiate Prefab"))
      {
        for (const char *path : kKnownPrefabs)
        {
          if (ImGui::MenuItem(path))
          {
            m_instantiatePrefab(path);
          }
        }
        ImGui::EndMenu();
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
