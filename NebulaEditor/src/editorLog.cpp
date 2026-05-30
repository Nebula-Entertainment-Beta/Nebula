#include "editorLog.h"
#include <imgui.h>

namespace Editor
{

  void EditorLog::draw()
  {

    for (const std::string &line : lines)
    {
      ImGui::TextUnformatted(line.c_str());
    }
  }

  void EditorLog::info(std::string_view msg)
  {
    lines.push_back(std::string(msg));
  }

}