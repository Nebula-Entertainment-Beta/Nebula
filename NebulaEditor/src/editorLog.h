#pragma once
#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

#include "logSink.h"

namespace Editor
{
  struct LogLine
  {
    Nebula::LogLevel level = Nebula::LogLevel::Info;
    std::string text;
  };

  class EditorLog
  {
  public:
    static constexpr std::size_t kMaxLines = 1000;

    void info(std::string_view msg);
    void warn(std::string_view msg);
    void error(std::string_view msg);
    void log(Nebula::LogLevel level, std::string_view msg);
    void clear();
    const std::vector<LogLine> &getLines() const { return lines; }

  private:
    void push(Nebula::LogLevel level, std::string_view msg);
    std::vector<LogLine> lines;
  };

}
