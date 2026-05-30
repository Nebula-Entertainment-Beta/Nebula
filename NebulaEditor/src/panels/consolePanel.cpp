#include "consolePanel.h"
#include "editorLog.h"
#include <imgui.h>

namespace Editor
{

  void ConsolePanel::drawConsolePanel(EditorLog &log)
  {
    ImGui::Begin("Console");
    // create scrolable child
    ImGui::BeginChild("ConsoleChild", ImVec2(0, 0), true);
    log.draw();

    ImGui::EndChild();
    ImGui::End();
  }
}