#pragma once
#include <string_view>
#include <vector>

namespace Editor
{
  class EditorLog
  {
  public:
    void info(std::string_view msg);
    void draw();

  private:
    std::vector<std::string> lines;
  };

}