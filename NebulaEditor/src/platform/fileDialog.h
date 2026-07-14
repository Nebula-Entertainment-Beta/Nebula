#pragma once

#include <string>
#include <vector>

namespace Editor
{

  /** Opens a native file picker. Returns empty if cancelled. */
  std::string openFileDialog(const char *title, const std::vector<std::string> &extensions);

  /** Opens a native save picker. Returns empty if cancelled. */
  std::string saveFileDialog(const char *title, const std::string &defaultName,
                             const std::vector<std::string> &extensions);

} // namespace Editor
