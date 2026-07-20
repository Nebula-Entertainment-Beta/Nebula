#include "editorLog.h"

namespace Editor
{

  void EditorLog::push(Nebula::LogLevel level, std::string_view msg)
  {
    lines.push_back(LogLine{level, std::string(msg)});
    if (lines.size() > kMaxLines)
    {
      lines.erase(lines.begin(), lines.begin() + static_cast<std::ptrdiff_t>(lines.size() - kMaxLines));
    }
  }

  void EditorLog::info(std::string_view msg)
  {
    push(Nebula::LogLevel::Info, msg);
  }

  void EditorLog::warn(std::string_view msg)
  {
    push(Nebula::LogLevel::Warn, msg);
  }

  void EditorLog::error(std::string_view msg)
  {
    push(Nebula::LogLevel::Error, msg);
  }

  void EditorLog::log(Nebula::LogLevel level, std::string_view msg)
  {
    push(level, msg);
  }

  void EditorLog::clear()
  {
    lines.clear();
  }

}
