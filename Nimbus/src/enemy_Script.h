#pragma once
#include <script.h>
#include "combat.h"
#include "scriptParams.h"

namespace Nimbus
{

  enum class EnemyStates
  {
    Idle,
    Chase,
    Windup,
    Attack,
    Stagger,
    Knockback,
    Dead,
  };

  class enemyScript : public Nebula::IScript
  {
  public:
    void onCreate(Nebula::ScriptContext &, Nebula::Entity) override;
    void onEnable(Nebula::ScriptContext &, Nebula::Entity) override {}
    void onUpdate(Nebula::ScriptContext &, Nebula::Entity, float) override;
    void onPhysicsUpdate(Nebula::ScriptContext &, Nebula::Entity, float) override {}
    void EnemyCombatFSM(Nebula::ScriptContext &, Nebula::Entity, float, EnemyStates);
    void onRender(Nebula::ScriptContext &, Nebula::Entity, float) override {}
    void onDisable(Nebula::ScriptContext &, Nebula::Entity) override {}
    void onDestroy(Nebula::ScriptContext &, Nebula::Entity) override {}
    EnemyStates setEnemyState(EnemyStates state);
    EnemyStates getEnemyState() const;
    void receiveHit(Nebula::ScriptContext &ctx, Nebula::Entity self, float damage, bool heavy);

  private:
    Nebula::Entity findPlayer(Nebula::ScriptContext &ctx) const;
    float distanceTo(Nebula::ScriptContext &ctx, Nebula::Entity self, Nebula::Entity target) const;
    void moveToward(Nebula::ScriptContext &ctx, Nebula::Entity self, Nebula::Entity target, float dt,
                    float speed);

    EnemyStates m_AttackState = EnemyStates::Idle;
    float attackDamage = 10.f;
    float health = 100.f;
    float stateTimer = 0.f;
    float m_attackCooldownTimer = 0.f;
    bool m_attackDealt = false;
    Nebula::Vec3 m_knockbackDir{};
    Nebula::ScriptParams m_params;
  };

}
