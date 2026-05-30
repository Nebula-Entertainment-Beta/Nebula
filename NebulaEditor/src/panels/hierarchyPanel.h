#pragma once
#include "editorState.h"
#include "Nebula.h"

namespace Editor
{
  class HierarchyPanel
  {
  public:
    void drawHierarchyPanel(Nebula::Scene &scene, EditorState &state);
  };
}