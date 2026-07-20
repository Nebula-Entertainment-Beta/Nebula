#pragma once

#include <string_view>

namespace Nebula
{

  enum class LogLevel
  {
    Info,
    Warn,
    Error
  };

  /** Optional script logging; host (e.g. editor) may wire this to a console panel. */
  class ILogSink
  {
  public:
    virtual ~ILogSink() = default;
    virtual void info(std::string_view msg) = 0;
    virtual void warn(std::string_view msg) { info(msg); }
    virtual void error(std::string_view msg) { info(msg); }
    virtual void log(LogLevel level, std::string_view msg)
    {
      switch (level)
      {
      case LogLevel::Warn:
        warn(msg);
        break;
      case LogLevel::Error:
        error(msg);
        break;
      case LogLevel::Info:
      default:
        info(msg);
        break;
      }
    }
  };

}
