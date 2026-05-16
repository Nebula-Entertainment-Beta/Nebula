#include "clock.h"
#include <chrono>

namespace Nebula
{

  double clock::nowSeconds() const
  {
    static const auto startTime = std::chrono::steady_clock::now();
    auto currentTime = std::chrono::steady_clock::now();

    std::chrono::duration<double> elapsed = currentTime - startTime;
    return elapsed.count();
  }

}
