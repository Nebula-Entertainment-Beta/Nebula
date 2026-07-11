#pragma once

#include "editorState.h"
#include "Nebula.h"
#include "scriptFields.h"
#include "scriptParams.h"

namespace Editor
{
  class DebugPanel
  {
  public:
    void drawDebugPanel(EditorState &state, Nebula::Scene &scene, Nebula::Input &input,
                        Nebula::ActionMapping &actions, Nebula::ScriptFieldRegistry &fieldRegistry,
                        bool isPlaying);

  private:
    void drawInputOverlay(Nebula::Input &input, Nebula::ActionMapping &actions);
    void drawCombatTuning(Nebula::Scene &scene, Nebula::ScriptFieldRegistry &fieldRegistry,
                          EditorState &state, bool isPlaying);

    Nebula::Entity findCombatDirectorEntity(Nebula::Scene &scene) const;
    Nebula::ScriptParams m_scriptParams;
  };
}
