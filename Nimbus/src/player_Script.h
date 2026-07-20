#pragma once
#include <script.h>
#include <scriptParams.h>
#include "ecs/entity.h"

namespace Nimbus
{
  enum AttackStates
  {
    Idle,
    LightWindup,
    HeavyWindup,
    ActiveHitLight,
    ActiveHitHeavy,
    RecoveryLightAttack,
    RecoveryHeavyAttack,
  };

  class PlayerScript : public Nebula::IScript
  {
  public:
    void onCreate(Nebula::ScriptContext &ctx, Nebula::Entity self) override;
    void onEnable(Nebula::ScriptContext &, Nebula::Entity) override {}
    void onUpdate(Nebula::ScriptContext &, Nebula::Entity, float) override;
    void movement(Nebula::ScriptContext &, Nebula::Entity, float fixedDt);
    void combatFSM(Nebula::ScriptContext &, Nebula::Entity, float, AttackStates state);
    void onPhysicsUpdate(Nebula::ScriptContext &, Nebula::Entity, float fixedDt) override;
    void onRender(Nebula::ScriptContext &, Nebula::Entity, float) override {}
    void onDisable(Nebula::ScriptContext &, Nebula::Entity) override {}
    void onDestroy(Nebula::ScriptContext &, Nebula::Entity) override {}
    AttackStates getAttackState();
    AttackStates setAttackStates(AttackStates state);
    Nebula::Entity GetCamera(Nebula::ScriptContext &ctx, Nebula::Entity self);
    float getMoveSpeedMultiplier() const;
    bool isInvulnerable() const { return m_playerIFrameTimer > 0.f; }
    /** Call when an enemy hit lands (Day 4). Duration from CombatDirector tuning. */
    void grantIFrame(Nebula::ScriptContext &ctx);

  private:
    void applyPendingPlayerDamage(Nebula::ScriptContext &ctx);
    void applyAttackLunge(Nebula::ScriptContext &ctx, Nebula::Entity self, float dt, float speedMultiplier);
    void snapToGround(Nebula::ScriptContext &ctx, Nebula::Entity self);
    void resolveActiveHit(Nebula::ScriptContext &ctx, Nebula::Entity self, float damage, bool heavy,
                          const char *label);

    float m_moveSpeed = 3.f;
    float m_health = 100.f;
    float stateTimer = 0.f;
    float m_playerIFrameTimer = 0.f;
    bool m_hitThisSwing = false;
    bool m_grounded = false;

    float m_velocityY = 0.f;
    float m_coyoteTimer = 0.f;
    float m_jumpBufferTimer = 0.f;
    float m_jumpGraceTimer = 0.f;
    bool m_wantsJump = false;
    bool m_prevJumpHeld = false;
    bool m_pendingJumpCut = false;
    bool m_pendingGroundSnap = true;
    Nebula::Vec3 m_spawnPosition{0.f, 0.5f, 0.f};

    AttackStates m_AttackState;
    Nebula::ScriptParams m_params;
  };

}
