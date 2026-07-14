#include "platform/fileDialog.h"

namespace Editor
{

  std::string openFileDialog(const char * /*title*/, const std::vector<std::string> & /*extensions*/)
  {
    return {};
  }

  std::string saveFileDialog(const char * /*title*/, const std::string & /*defaultName*/,
                             const std::vector<std::string> & /*extensions*/)
  {
    return {};
  }

} // namespace Editor
