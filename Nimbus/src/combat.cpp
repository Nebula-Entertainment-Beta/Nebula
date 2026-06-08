#include "combat.h"
#include <algorithm>

namespace Nimbus
{

  Combat &Combat::instance()
  {
    static Combat combat;
    return combat;
  }
  int Combat::wavesPerSecond() const
  {
    return std::clamp(enemiesPerWave, 3, 8);
  }
}