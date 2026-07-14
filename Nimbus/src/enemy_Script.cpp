#include "enemy_Script.h"
#include "nimbus_config.h"

#include <cmath>
#include <string>

namespace Nimbus
{
  void enemyScript::onCreate(Nebula::ScriptContext &ctx, Nebula::Entity entity)
  {
    setEnemyState(EnemyStates::Idle);
    if (!ctx.scene.isValidEntity(entity))
    {
      return;
    }
    const Nebula::ScriptComponent &sc = ctx.scene.getScriptComponent(entity);
    const float tunedHealth = Nimbus::Combat::instance().enemyMaxHealth;
    health = m_params.readScriptParamFloat(sc.paramsJson, "health", tunedHealth);
    attackDamage = m_params.readScriptParamFloat(sc.paramsJson, "attackDamage", 10.f);
  }

  void enemyScript::onUpdate(Nebula::ScriptContext &ctx, Nebula::Entity entity, float dt)
  {
    float hitDamage = 0.f;
    bool hitHeavy = false;
    while (Nimbus::Combat::instance().popEnemyHit(entity, hitDamage, hitHeavy))
    {
      receiveHit(ctx, entity, hitDamage, hitHeavy);
    }

    if (m_attackCooldownTimer > 0.f)
    {
      m_attackCooldownTimer -= dt;
    }
    EnemyCombatFSM(ctx, entity, dt, getEnemyState());
  }

  Nebula::Entity enemyScript::findPlayer(Nebula::ScriptContext &ctx) const
  {
    return ctx.scene.findByTag(kPlayerTag);
  }

  float enemyScript::distanceTo(Nebula::ScriptContext &ctx, Nebula::Entity self,
                                Nebula::Entity target) const
  {
    if (!ctx.scene.isValidEntity(self) || !ctx.scene.isValidEntity(target))
    {
      return 1e9f;
    }
    const Nebula::Vec3 a = ctx.scene.getTransform(self).transform.getPosition();
    const Nebula::Vec3 b = ctx.scene.getTransform(target).transform.getPosition();
    const float dx = b.x - a.x;
    const float dz = b.z - a.z;
    return std::sqrt(dx * dx + dz * dz);
  }

  void enemyScript::moveToward(Nebula::ScriptContext &ctx, Nebula::Entity self, Nebula::Entity target,
                               float dt, float speed)
  {
    if (!ctx.scene.isValidEntity(self) || !ctx.scene.isValidEntity(target))
    {
      return;
    }
    auto &transform = ctx.scene.getTransform(self);
    Nebula::Vec3 pos = transform.transform.getPosition();
    const Nebula::Vec3 targetPos = ctx.scene.getTransform(target).transform.getPosition();
    float dx = targetPos.x - pos.x;
    float dz = targetPos.z - pos.z;
    const float len = std::sqrt(dx * dx + dz * dz);
    if (len < 1e-4f)
    {
      return;
    }
    dx /= len;
    dz /= len;
    pos.x += dx * speed * dt;
    pos.z += dz * speed * dt;
    transform.transform.setPosition(pos);
  }

  void enemyScript::receiveHit(Nebula::ScriptContext &ctx, Nebula::Entity self, float damage,
                               bool heavy)
  {
    if (m_AttackState == EnemyStates::Dead)
    {
      return;
    }
    health -= damage;
    if (health <= 0.f)
    {
      setEnemyState(EnemyStates::Dead);
      return;
    }
    if (heavy)
    {
      const Nebula::Entity player = findPlayer(ctx);
      if (ctx.scene.isValidEntity(player))
      {
        const Nebula::Vec3 enemyPos = ctx.scene.getTransform(self).transform.getPosition();
        const Nebula::Vec3 playerPos = ctx.scene.getTransform(player).transform.getPosition();
        float dx = enemyPos.x - playerPos.x;
        float dz = enemyPos.z - playerPos.z;
        const float len = std::sqrt(dx * dx + dz * dz);
        if (len > 1e-4f)
        {
          m_knockbackDir = {dx / len, 0.f, dz / len};
        }
      }
      setEnemyState(EnemyStates::Knockback);
    }
    else
    {
      setEnemyState(EnemyStates::Stagger);
    }
  }

  EnemyStates enemyScript::setEnemyState(EnemyStates state)
  {
    m_AttackState = state;
    stateTimer = 0.f;
    if (state == EnemyStates::Windup)
    {
      m_attackDealt = false;
    }
    return m_AttackState;
  }

  EnemyStates enemyScript::getEnemyState() const
  {
    return m_AttackState;
  }

  void enemyScript::EnemyCombatFSM(Nebula::ScriptContext &ctx, Nebula::Entity entity, float dt,
                                   EnemyStates state)
  {
    if (m_AttackState == EnemyStates::Dead)
    {
      return;
    }

    const Nimbus::Combat &t = Nimbus::Combat::instance();
    const Nebula::Entity player = findPlayer(ctx);
    const float dist = distanceTo(ctx, entity, player);

    switch (state)
    {
    case EnemyStates::Idle:
      if (ctx.scene.isValidEntity(player) && dist <= t.aggroRange)
      {
        setEnemyState(EnemyStates::Chase);
      }
      break;

    case EnemyStates::Chase:
      if (!ctx.scene.isValidEntity(player) || dist > t.aggroRange)
      {
        setEnemyState(EnemyStates::Idle);
        break;
      }
      if (dist <= t.enemyAttackRange && m_attackCooldownTimer <= 0.f)
      {
        if (ctx.scene.isValidEntity(entity))
        {
          m_windupBaseScale = ctx.scene.getTransform(entity).transform.getScale();
        }
        setEnemyState(EnemyStates::Windup);
        break;
      }
      if (dist > t.enemyAttackRange)
      {
        moveToward(ctx, entity, player, dt, t.enemyMoveSpeed);
      }
      break;

    case EnemyStates::Windup:
      stateTimer += dt;
      if (ctx.scene.isValidEntity(entity))
      {
        auto &transform = ctx.scene.getTransform(entity);
        const float pulse = 1.f + 0.12f * std::sin(stateTimer * 24.f);
        transform.transform.setScale(
            {m_windupBaseScale.x * pulse, m_windupBaseScale.y * pulse, m_windupBaseScale.z * pulse});
        Nebula::Vec3 pos = transform.transform.getPosition();
        pos.y = 0.5f + 0.06f * std::sin(stateTimer * 24.f);
        transform.transform.setPosition(pos);
      }
      if (stateTimer >= t.lightWindup)
      {
        if (ctx.scene.isValidEntity(entity))
        {
          auto &transform = ctx.scene.getTransform(entity);
          transform.transform.setScale(m_windupBaseScale);
          Nebula::Vec3 pos = transform.transform.getPosition();
          pos.y = 0.5f;
          transform.transform.setPosition(pos);
        }
        setEnemyState(EnemyStates::Attack);
      }
      break;

    case EnemyStates::Attack:
      if (!m_attackDealt)
      {
        if (ctx.scene.isValidEntity(player) && dist <= t.enemyAttackRange)
        {
          if (Nimbus::Combat::instance().playerIFrameTimer <= 0.f)
          {
            Nimbus::Combat::instance().queuePlayerDamage(attackDamage);
          }
          if (ctx.log != nullptr)
          {
            ctx.log->info("[Combat] Enemy id=" + std::to_string(entity.id) + " attacked player for " +
                          std::to_string(attackDamage));
          }
        }
        m_attackDealt = true;
        m_attackCooldownTimer = t.enemyAttackInterval;
      }
      stateTimer += dt;
      if (stateTimer >= t.lightActive)
      {
        setEnemyState(EnemyStates::Chase);
      }
      break;

    case EnemyStates::Stagger:
      stateTimer += dt;
      if (stateTimer >= t.staggerDuration)
      {
        setEnemyState(EnemyStates::Chase);
      }
      break;

    case EnemyStates::Knockback:
      stateTimer += dt;
      if (ctx.scene.isValidEntity(entity))
      {
        auto &transform = ctx.scene.getTransform(entity);
        Nebula::Vec3 pos = transform.transform.getPosition();
        pos.x += m_knockbackDir.x * t.knockbackForce * dt;
        pos.z += m_knockbackDir.z * t.knockbackForce * dt;
        transform.transform.setPosition(pos);
      }
      if (stateTimer >= t.staggerDuration)
      {
        setEnemyState(EnemyStates::Chase);
      }
      break;

    case EnemyStates::Dead:
      stateTimer += dt;
      if (stateTimer >= 0.3f && ctx.scene.isValidEntity(entity))
      {
        ctx.scene.getTransform(entity).transform.setScale({0.f, 0.f, 0.f});
      }
      break;
    }
  }
}
