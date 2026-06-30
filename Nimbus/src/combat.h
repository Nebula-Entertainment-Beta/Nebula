#pragma once

#include "ecs/entity.h"
#include "frameCommands.h"

#include <vector>

namespace Nimbus
{

  struct EnemyHitRequest
  {
    Nebula::Entity target{};
    float damage = 0.f;
    bool heavy = false;
  };

  struct Combat
  {
    float lightDamage = 10.f;
    float heavyDamage = 25.f;
    float lightWindup = 0.08f;
    float lightActive = 0.12f;
    float lightRecovery = 0.2f;
    float heavyWindup = 0.18f;
    float heavyActive = 0.15f;
    float heavyRecovery = 0.35f;
    float hitRadius = 5.f;
    float playerIFrameDuration = 0.4f;
    // Enemy
    float enemyMaxHealth = 120.f;
    float knockbackForce = 400.f;
    float staggerDuration = 0.35f;
    float enemyAttackInterval = 1.5f;
    float enemyMoveSpeed = 2.f;
    float aggroRange = 12.f;
    float enemyAttackRange = 1.5f;

    int enemiesPerWave = 5; // clamp 3–8 in spawner
    float spawnRadius = 6.f;
    float timeBetweenWaves = 3.f;

    float playerIFrameTimer = 0.f;
    float pendingPlayerDamage = 0.f;
    std::vector<EnemyHitRequest> enemyHitRequests;

    static Combat &instance();
    int wavesPerSecond() const;
    void queueEnemyHit(Nebula::Entity target, float damage, bool heavy);
    bool popEnemyHit(Nebula::Entity target, float &damage, bool &heavy);
    void queuePlayerDamage(float damage);
    float consumePendingPlayerDamage();
  };

}