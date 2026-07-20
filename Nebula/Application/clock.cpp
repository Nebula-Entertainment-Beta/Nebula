#include "clock.h"

#include <chrono>

namespace Nebula
{
  namespace
  {
    double steadyNowSeconds()
    {
      using clock_t = std::chrono::steady_clock;
      return std::chrono::duration<double>(clock_t::now().time_since_epoch()).count();
    }
  }

  clock::clock() : m_startSeconds(steadyNowSeconds()) {}

  double clock::nowSeconds() const
  {
    return steadyNowSeconds() - m_startSeconds;
  }

}
