#pragma once
#include "editorState.h"
#include "Nebula.h"
#include <functional>
#include <string_view>

namespace Editor
{
  class HierarchyPanel
  {
  public:
    void setEntityActions(std::function<void(const char *templateId)> factory, std::function<void()> deleteEntity);
    void setPrefabActions(std::function<void(std::string_view prefabPath)> instantiatePrefab,
                          std::function<void()> saveSelectedAsPrefab);
    void drawHierarchyPanel(Nebula::Scene &scene, EditorState &state);

  private:
    std::function<void(const char *templateId)> m_entityFactory;
    std::function<void()> m_deleteEntity;
    std::function<void(std::string_view prefabPath)> m_instantiatePrefab;
    std::function<void()> m_saveSelectedAsPrefab;
  };
}
