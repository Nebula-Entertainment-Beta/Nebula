#include "player_Script.h"
#include "combatHelper.h"
#include "nimbus_config.h"
#include "physicsQuery.h"
#include "scriptParams.h"
#include "traversalVolumes.h"

#include <cmath>
#include <string>

namespace Nimbus
{

  void PlayerScript::onCreate(Nebula::ScriptContext &ctx, Nebula::Entity self)
  {
    setAttackStates(Idle);
    if (!ctx.scene.isValidEntity(self))
    {
      return;
    }
    const Nebula::ScriptComponent &sc = ctx.scene.getScriptComponent(self);
    m_moveSpeed = m_params.readScriptParamFloat(sc.paramsJson, "moveSpeed", 3.f);
    m_traversalDirector = ctx.scene.findByTag("TraversalDirector");
    m_spawnPosition = ctx.scene.getTransform(self).transform.getPosition();
    m_pendingGroundSnap = true;
  }

  void PlayerScript::snapToGround(Nebula::ScriptContext &ctx, Nebula::Entity self)
  {
    if (ctx.physics == nullptr || ctx.physicsScene == nullptr)
    {
      return;
    }

    auto &transform = ctx.scene.getTransform(self).transform;
    Nebula::Vec3 pos = transform.getPosition();
    const float halfHeight = 0.5f * transform.getScale();

    Nebula::RaycastHit hit{};
    const Nebula::Vec3 origin{pos.x, pos.y - halfHeight + 0.01f, pos.z};
    const Nebula::Vec3 down{0.f, -1.f, 0.f};
    if (ctx.physics->raycast(*ctx.physicsScene, origin, down, 8.f, hit) && hit.entity != self &&
        hit.normal.y > 0.5f)
    {
      constexpr float kSkin = 0.02f;
      pos.y = hit.point.y + halfHeight + kSkin;
      transform.setPosition(pos);
      m_grounded = true;
      m_velocityY = 0.f;
      m_spawnPosition = pos;
      return;
    }

    for (int step = 0; step < 32; ++step)
    {
      if (ctx.physics->isGrounded(*ctx.physicsScene, self))
      {
        m_grounded = true;
        m_velocityY = 0.f;
        m_spawnPosition = ctx.scene.getTransform(self).transform.getPosition();
        return;
      }

      bool grounded = false;
      ctx.physics->moveKinematic(*ctx.physicsScene, self, Nebula::Vec3{0.f, -0.05f, 0.f}, grounded);
      if (grounded)
      {
        m_grounded = true;
        m_velocityY = 0.f;
        m_spawnPosition = ctx.scene.getTransform(self).transform.getPosition();
        return;
      }
    }
  }

  Nebula::Entity PlayerScript::GetCamera(Nebula::ScriptContext &ctx, Nebula::Entity self)
  {
    (void)self;
    return ctx.scene.findByTag(Nimbus::kMainCameraTag);
  }

  void PlayerScript::onUpdate(Nebula::ScriptContext &ctx, Nebula::Entity self, float dt)
  {
    if (m_playerIFrameTimer > 0.f)
    {
      m_playerIFrameTimer -= dt;
      if (m_playerIFrameTimer < 0.f)
      {
        m_playerIFrameTimer = 0.f;
      }
    }
    Nimbus::Combat::instance().playerIFrameTimer = m_playerIFrameTimer;
    applyPendingPlayerDamage(ctx);

    // Capture jump on the render frame (edge + buffer) so presses aren't lost.
    if (ctx.input != nullptr)
    {
      const Nebula::FrameInput &f = ctx.input->frame();
      syncTraversalSettings(ctx);
      if (f.jumpPressed || (f.jumpHeld && !m_prevJumpHeld))
      {
        m_wantsJump = true;
        m_jumpBufferTimer = m_traversal.jumpBufferTime;
      }
      if (!f.jumpHeld && m_prevJumpHeld && m_velocityY > 0.f)
      {
        m_pendingJumpCut = true;
      }
      m_prevJumpHeld = f.jumpHeld;
    }

    combatFSM(ctx, self, dt, getAttackState());
  }

  void PlayerScript::onPhysicsUpdate(Nebula::ScriptContext &ctx, Nebula::Entity self, float fixedDt)
  {
    if (m_pendingGroundSnap)
    {
      snapToGround(ctx, self);
      m_pendingGroundSnap = false;
    }
    movement(ctx, self, fixedDt);
  }

  void PlayerScript::grantIFrame()
  {
    m_playerIFrameTimer = Nimbus::Combat::instance().playerIFrameDuration;
    Nimbus::Combat::instance().playerIFrameTimer = m_playerIFrameTimer;
  }

  void PlayerScript::applyPendingPlayerDamage(Nebula::ScriptContext &ctx)
  {
    const float damage = Nimbus::Combat::instance().consumePendingPlayerDamage();
    if (damage <= 0.f || isInvulnerable())
    {
      return;
    }
    m_health -= damage;
    grantIFrame();
    if (ctx.log != nullptr)
    {
      ctx.log->info("[Combat] Player took " + std::to_string(damage) +
                    " damage, health=" + std::to_string(m_health));
    }
  }

  void PlayerScript::syncTraversalSettings(Nebula::ScriptContext &ctx)
  {
    if (!ctx.scene.isValidEntity(m_traversalDirector))
      return;
    syncTraversalFromParams(
        ctx.scene.getScriptComponent(m_traversalDirector),
        m_params,
        m_traversal);
  }

  void PlayerScript::movement(Nebula::ScriptContext &ctx, Nebula::Entity self, float fixedDt)
  {
    if (!ctx.scene.isValidEntity(self) || ctx.physics == nullptr ||
        ctx.physicsScene == nullptr || ctx.input == nullptr)
      return;

    const Nebula::Entity cameraEntity = GetCamera(ctx, self);
    if (!ctx.scene.isValidEntity(cameraEntity))
      return;

    syncTraversalSettings(ctx);
    const TraversalSettings &t = m_traversal;
    const Nebula::FrameInput &f = ctx.input->frame();

    const bool wasGrounded = m_grounded;

    bool onGround = m_grounded;
    if (!onGround)
    {
      onGround = ctx.physics->isGrounded(*ctx.physicsScene, self);
    }

    // --- Try jump (buffer / edge + coyote or grounded) ---
    const bool canJump = onGround || m_coyoteTimer > 0.f;
    const bool jumpRequested = m_wantsJump || m_jumpBufferTimer > 0.f || f.jumpPressed;
    if (jumpRequested && canJump)
    {
      m_velocityY = t.jumpSpeed;
      m_jumpBufferTimer = 0.f;
      m_coyoteTimer = 0.f;
      m_grounded = false;
      m_wantsJump = false;
      m_jumpGraceTimer = 0.15f;
      if (ctx.log != nullptr)
      {
        ctx.log->info("[Traversal] Player jump (speed=" + std::to_string(t.jumpSpeed) + ")");
      }
    }

    // --- Gravity + fast fall ---
    if (!m_grounded)
    {
      float grav = t.gravity;
      if (f.fastFall)
        grav *= t.fastFallMult;

      m_velocityY -= grav * fixedDt;
      m_velocityY = std::clamp(m_velocityY, -t.terminalVelocity, t.terminalVelocity);
    }

    if (m_pendingJumpCut)
    {
      m_velocityY *= 0.5f;
      m_pendingJumpCut = false;
    }

    const VolumeQueryContext volumeCtx{ctx.physics, ctx.physicsScene};
    const float bounce = queryBounceImpulseIfOverlapping(volumeCtx, self, m_velocityY);
    if (bounce > 0.f)
    {
      m_velocityY = bounce;
      m_grounded = false;
      m_coyoteTimer = 0.f;
    }

    const float wind = queryWindLiftIfOverlapping(volumeCtx, self);
    if (wind > 0.f)
    {
      m_velocityY = std::max(m_velocityY, wind);
    }

    // --- Horizontal (camera-relative) ---
    Nebula::Vec3 moveDir{f.moveX, 0.f, f.moveY};
    if (moveDir.x != 0.f || moveDir.z != 0.f)
    {
      const float len = std::sqrt(moveDir.x * moveDir.x + moveDir.z * moveDir.z);
      moveDir.x /= len;
      moveDir.z /= len;
    }

    const float yaw = ctx.scene.getCamera(cameraEntity).yaw;
    const Nebula::Vec3 forward{-std::sin(yaw), 0.f, -std::cos(yaw)};
    const Nebula::Vec3 right{std::cos(yaw), 0.f, -std::sin(yaw)};

    const float controlMult = m_grounded ? 1.f : t.airControlMult;
    const float speed = m_moveSpeed * getMoveSpeedMultiplier() * controlMult;

    Nebula::Vec3 delta{
        forward.x * moveDir.z + right.x * moveDir.x,
        0.f,
        forward.z * moveDir.z + right.z * moveDir.x};
    delta = delta * (speed * fixedDt);
    delta.y = m_velocityY * fixedDt;

    if (m_grounded && m_velocityY <= 0.f && m_jumpGraceTimer <= 0.f)
    {
      constexpr float kGroundStick = 0.08f;
      delta.y = std::min(delta.y, -kGroundStick);
    }

    // Resolve vertical then horizontal so floor contact doesn't eat jump motion.
    bool groundedFromVertical = false;
    bool groundedFromHorizontal = false;
    const Nebula::Vec3 verticalDelta{0.f, delta.y, 0.f};
    const Nebula::Vec3 horizontalDelta{delta.x, 0.f, delta.z};

    if (verticalDelta.y != 0.f)
    {
      ctx.physics->moveKinematic(*ctx.physicsScene, self, verticalDelta, groundedFromVertical);
    }

    if (horizontalDelta.x != 0.f || horizontalDelta.z != 0.f)
    {
      ctx.physics->moveKinematic(*ctx.physicsScene, self, horizontalDelta, groundedFromHorizontal);
    }

    const bool contactGround = groundedFromVertical || groundedFromHorizontal;
    if (m_jumpGraceTimer > 0.f)
    {
      m_jumpGraceTimer = std::max(0.f, m_jumpGraceTimer - fixedDt);
      m_grounded = false;
    }
    else
    {
      m_grounded = contactGround && m_velocityY <= 0.f;
    }

    m_jumpBufferTimer = std::max(0.f, m_jumpBufferTimer - fixedDt);

    // --- Land / coyote ---
    if (m_grounded && m_velocityY <= 0.f)
      m_velocityY = 0.f;

    if (m_grounded)
      m_coyoteTimer = t.coyoteTime;
    else if (wasGrounded)
      m_coyoteTimer = t.coyoteTime; // just walked off ledge
    else
      m_coyoteTimer = std::max(0.f, m_coyoteTimer - fixedDt);

    // --- Kill plane ---
    const Nebula::Vec3 pos = ctx.scene.getTransform(self).transform.getPosition();
    if (pos.y < t.killY)
    {
      auto &xf = ctx.scene.getTransform(self);
      xf.transform.setPosition(m_spawnPosition);
      m_velocityY = 0.f;
      m_grounded = false;
      m_coyoteTimer = 0.f;
      m_jumpBufferTimer = 0.f;
    }
  }

  void PlayerScript::applyAttackLunge(Nebula::ScriptContext &ctx, Nebula::Entity self, float dt, float speedMultiplier)
  {
    if (!ctx.scene.isValidEntity(self))
    {
      return;
    }
    const Nebula::Entity cameraEntity = GetCamera(ctx, self);
    if (!ctx.scene.isValidEntity(cameraEntity))
    {
      return;
    }
    const float yaw = ctx.scene.getCamera(cameraEntity).yaw;
    const Nebula::Vec3 forward{-std::sin(yaw), 0.0f, -std::cos(yaw)};
    if (ctx.physics != nullptr && ctx.physicsScene != nullptr)
    {
      bool grounded = false;
      const Nebula::Vec3 delta = forward * (m_moveSpeed * speedMultiplier * dt);
      ctx.physics->moveKinematic(*ctx.physicsScene, self, delta, grounded);
    }
    else
    {
      auto &transformComponent = ctx.scene.getTransform(self);
      Nebula::Vec3 pos = transformComponent.transform.getPosition();
      pos += forward * (m_moveSpeed * speedMultiplier * dt);
      transformComponent.transform.setPosition(pos);
    }
  }

  float PlayerScript::getMoveSpeedMultiplier() const
  {
    switch (m_AttackState)
    {
    case AttackStates::LightWindup:
    case AttackStates::HeavyWindup:
      return 0.25f;
    case AttackStates::ActiveHitLight:
    case AttackStates::LightAttack:
      return 0.25f;
    case AttackStates::ActiveHitHeavy:
    case AttackStates::HeavyAttack:
      return 0.0f;
    case AttackStates::RecoveryLightAttack:
    case AttackStates::RecoveryHeavyAttack:
      return 0.5f;
    default:
      return 1.0f;
    }
  }

  void PlayerScript::combatFSM(Nebula::ScriptContext &ctx, Nebula::Entity self, float dt, AttackStates state)
  {
    if (ctx.input == nullptr)
    {
      return;
    }
    const Nebula::FrameInput &f = ctx.input->frame();
    const Nimbus::Combat &t = Nimbus::Combat::instance();

    switch (state)
    {
    case AttackStates::Idle:
      if (f.lightAttackPressed)
      {
        setAttackStates(AttackStates::LightWindup);
      }
      else if (f.heavyAttackPressed)
      {
        setAttackStates(AttackStates::HeavyWindup);
      }
      break;

    case AttackStates::LightWindup:
      stateTimer += dt;
      if (stateTimer >= t.lightWindup)
      {
        setAttackStates(AttackStates::ActiveHitLight);
      }
      break;

    case AttackStates::HeavyWindup:
      stateTimer += dt;
      if (stateTimer >= t.heavyWindup)
      {
        setAttackStates(AttackStates::ActiveHitHeavy);
      }
      break;

    case AttackStates::ActiveHitLight:
    case AttackStates::LightAttack:
      stateTimer += dt;
      applyAttackLunge(ctx, self, dt, 0.5f);
      if (!m_hitThisSwing)
      {
        const Nebula::Entity cameraEntity = GetCamera(ctx, self);
        if (ctx.scene.isValidEntity(cameraEntity))
        {
          const float yaw = ctx.scene.getCamera(cameraEntity).yaw;
          const Nebula::Vec3 forward{-std::sin(yaw), 0.0f, -std::cos(yaw)};
          const Nebula::Vec3 playerPos = ctx.scene.getTransform(self).transform.getPosition();
          const Nebula::Vec3 hitCenter = playerPos + forward * 0.8f;
          const std::vector<Nebula::Entity> hits =
              findEnemiesInSphere(ctx.scene, hitCenter, t.hitRadius);
          if (ctx.log != nullptr)
          {
            if (hits.empty())
            {
              ctx.log->info("[Combat] Light attack: no hits (radius=" +
                            std::to_string(t.hitRadius) + ")");
            }
            else
            {
              for (const Nebula::Entity enemy : hits)
              {
                Nimbus::Combat::instance().queueEnemyHit(enemy, t.lightDamage, false);
                ctx.log->info("[Combat] Light attack hit enemy id=" +
                              std::to_string(enemy.id) + " damage=" +
                              std::to_string(t.lightDamage));
              }
            }
          }
        }
        m_hitThisSwing = true;
      }
      if (stateTimer >= t.lightActive)
      {
        setAttackStates(AttackStates::RecoveryLightAttack);
      }
      break;

    case AttackStates::ActiveHitHeavy:
    case AttackStates::HeavyAttack:
      stateTimer += dt;
      applyAttackLunge(ctx, self, dt, 1.25f);
      if (!m_hitThisSwing)
      {
        const Nebula::Entity cameraEntity = GetCamera(ctx, self);
        if (ctx.scene.isValidEntity(cameraEntity))
        {
          const float yaw = ctx.scene.getCamera(cameraEntity).yaw;
          const Nebula::Vec3 forward{-std::sin(yaw), 0.0f, -std::cos(yaw)};
          const Nebula::Vec3 playerPos = ctx.scene.getTransform(self).transform.getPosition();
          const Nebula::Vec3 hitCenter = playerPos + forward * 0.8f;
          const std::vector<Nebula::Entity> hits =
              findEnemiesInSphere(ctx.scene, hitCenter, t.hitRadius);
          if (ctx.log != nullptr)
          {
            if (hits.empty())
            {
              ctx.log->info("[Combat] Heavy attack: no hits (radius=" +
                            std::to_string(t.hitRadius) + ")");
            }
            else
            {
              for (const Nebula::Entity enemy : hits)
              {
                Nimbus::Combat::instance().queueEnemyHit(enemy, t.heavyDamage, true);
                ctx.log->info("[Combat] Heavy attack hit enemy id=" +
                              std::to_string(enemy.id) + " damage=" +
                              std::to_string(t.heavyDamage));
              }
            }
          }
        }
        m_hitThisSwing = true;
      }
      if (stateTimer >= t.heavyActive)
      {
        setAttackStates(AttackStates::RecoveryHeavyAttack);
      }
      break;

    case AttackStates::RecoveryLightAttack:
      stateTimer += dt;
      if (stateTimer >= t.lightRecovery)
      {
        setAttackStates(AttackStates::Idle);
      }
      break;

    case AttackStates::RecoveryHeavyAttack:
      stateTimer += dt;
      if (stateTimer >= t.heavyRecovery)
      {
        setAttackStates(AttackStates::Idle);
      }
      break;

    default:
      break;
    }
  }

  AttackStates PlayerScript::getAttackState()
  {
    return m_AttackState;
  }

  AttackStates PlayerScript::setAttackStates(AttackStates state)
  {
    m_AttackState = state;
    stateTimer = 0.f;
    if (state == AttackStates::LightWindup || state == AttackStates::HeavyWindup)
    {
      m_hitThisSwing = false;
    }
    return m_AttackState;
  }

}
