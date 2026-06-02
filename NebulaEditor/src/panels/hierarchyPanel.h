#pragma once
#include "editorState.h"
#include "Nebula.h"
#include <functional>

namespace Editor
{
  class HierarchyPanel
  {
  public:
    void setEntityActions(std::function<void(const char *templateId)> factory, std::function<void()> deleteEntity);
    void drawHierarchyPanel(Nebula::Scene &scene, EditorState &state);

  private:
    std::function<void(const char *templateId)> m_entityFactory;
    std::function<void()> m_deleteEntity;
  };
}
