#pragma once
#include "editorState.h"
#include "Nebula.h"
#include "scriptParams.h"
#include "scriptFields.h"

namespace Editor
{
  class sceneViewFrameBuffer;

  class InspectorPanel
  {

  public:
    void drawInspectorPanel(EditorState &state, Nebula::Scene &scene, Nebula::Entity entity, Nebula::ScriptFieldRegistry &scriptFieldRegistry);
    // script fields use existingregistry+ params json
    void drawScriptFields(Nebula::Entity entity, Nebula::ScriptFieldRegistry &fieldRegistry, Nebula::Scene &scene, EditorState &state);

  private:
    Nebula::ScriptParams m_scriptParams;
  };

}