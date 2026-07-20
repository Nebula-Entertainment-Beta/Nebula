#include "combat.h"
#include <algorithm>

namespace Nimbus
{

  int Combat::wavesPerSecond() const
  {
    return std::clamp(enemiesPerWave, 3, 8);
  }

  void Combat::queueEnemyHit(Nebula::Entity target, float damage, bool heavy)
  {
    enemyHitRequests.push_back({target, damage, heavy});
  }

  bool Combat::popEnemyHit(Nebula::Entity target, float &damage, bool &heavy)
  {
    for (auto it = enemyHitRequests.begin(); it != enemyHitRequests.end(); ++it)
    {
      if (it->target == target)
      {
        damage = it->damage;
        heavy = it->heavy;
        enemyHitRequests.erase(it);
        return true;
      }
    }
    return false;
  }

  void Combat::queuePlayerDamage(float damage)
  {
    pendingPlayerDamage = damage;
  }

  float Combat::consumePendingPlayerDamage()
  {
    const float damage = pendingPlayerDamage;
    pendingPlayerDamage = 0.f;
    return damage;
  }
}