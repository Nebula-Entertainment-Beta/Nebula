#pragma once

#include <string_view>

namespace Nebula
{

  /** Optional script logging; host (e.g. editor) may wire this to a console panel. */
  class ILogSink
  {
  public:
    virtual ~ILogSink() = default;
    virtual void info(std::string_view msg) = 0;
  };

}
