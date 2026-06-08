#include "player_Script.h"
#include "combatHelper.h"
#include "nimbus_config.h"
#include "scriptParams.h"

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
    movement(ctx, self, dt);
    combatFSM(ctx, self, dt, getAttackState());
  }

  void PlayerScript::grantIFrame()
  {
    m_playerIFrameTimer = Nimbus::Combat::instance().playerIFrameDuration;
  }

  void PlayerScript::movement(Nebula::ScriptContext &ctx, Nebula::Entity self, float dt)
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
    auto &cameraComponent = ctx.scene.getCamera(cameraEntity);
    auto &transformComponent = ctx.scene.getTransform(self);

    if (ctx.input == nullptr)
    {
      return;
    }
    const Nebula::FrameInput &f = ctx.input->frame();

    Nebula::Vec3 moveDir{f.moveX, 0.0f, f.moveY};

    if (moveDir.x != 0.0f || moveDir.z != 0.0f)
    {
      const float len = std::sqrt(moveDir.x * moveDir.x + moveDir.z * moveDir.z);
      moveDir.x /= len;
      moveDir.z /= len;
    }
    const float yaw = cameraComponent.yaw;

    const Nebula::Vec3 forward{-std::sin(yaw), 0.0f, -std::cos(yaw)};
    const Nebula::Vec3 right{std::cos(yaw), 0.0f, -std::sin(yaw)};

    Nebula::Vec3 velocity{
        forward.x * moveDir.z + right.x * moveDir.x,
        forward.y * moveDir.z + right.y * moveDir.x,
        forward.z * moveDir.z + right.z * moveDir.x};
    velocity = velocity * (m_moveSpeed * getMoveSpeedMultiplier() * dt);
    Nebula::Vec3 cubePos = transformComponent.transform.getPosition();
    cubePos += velocity;
    transformComponent.transform.setPosition(cubePos);
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
    auto &transformComponent = ctx.scene.getTransform(self);
    Nebula::Vec3 pos = transformComponent.transform.getPosition();
    pos += forward * (m_moveSpeed * speedMultiplier * dt);
    transformComponent.transform.setPosition(pos);
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
                ctx.log->info("[Combat] Light attack hit enemy id=" +
                              std::to_string(enemy.id) + " damage=" +
                              std::to_string(t.lightDamage));
                // Day 4: apply t.lightDamage to enemy
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
                ctx.log->info("[Combat] Heavy attack hit enemy id=" +
                              std::to_string(enemy.id) + " damage=" +
                              std::to_string(t.heavyDamage));
                // Day 4: apply t.heavyDamage to enemy
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
