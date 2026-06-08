#include "combatDirector.h"

namespace Nimbus
{
  void syncCombatFromParams(const Nebula::ScriptComponent &sc,
                            Nebula::ScriptParams &params,
                            Nimbus::Combat &combat,
                            int &enemiesPerWave,
                            float &spawnRadius,
                            float &timeBetweenWaves)
  {
    combat.lightDamage = params.readScriptParamFloat(sc.paramsJson, "lightDamage", 10.f);
    combat.heavyDamage = params.readScriptParamFloat(sc.paramsJson, "heavyDamage", 25.f);
    combat.lightWindup = params.readScriptParamFloat(sc.paramsJson, "lightWindup", 0.08f);
    combat.lightActive = params.readScriptParamFloat(sc.paramsJson, "lightActive", 0.12f);
    combat.lightRecovery = params.readScriptParamFloat(sc.paramsJson, "lightRecovery", 0.2f);
    combat.heavyWindup = params.readScriptParamFloat(sc.paramsJson, "heavyWindup", 0.18f);
    combat.heavyActive = params.readScriptParamFloat(sc.paramsJson, "heavyActive", 0.15f);
    combat.heavyRecovery = params.readScriptParamFloat(sc.paramsJson, "heavyRecovery", 0.35f);
    combat.hitRadius = params.readScriptParamFloat(sc.paramsJson, "hitRadius", 1.2f);
    combat.playerIFrameDuration = params.readScriptParamFloat(sc.paramsJson, "playerIFrameDuration", 0.4f);
    combat.enemyMaxHealth = params.readScriptParamFloat(sc.paramsJson, "enemyMaxHealth", 120.f);
    combat.knockbackForce = params.readScriptParamFloat(sc.paramsJson, "knockbackForce", 2.f);
    combat.staggerDuration = params.readScriptParamFloat(sc.paramsJson, "staggerDuration", 0.35f);
    combat.enemyAttackInterval = params.readScriptParamFloat(sc.paramsJson, "enemyAttackInterval", 1.5f);
    combat.enemyMoveSpeed = params.readScriptParamFloat(sc.paramsJson, "enemyMoveSpeed", 2.f);
    combat.aggroRange = params.readScriptParamFloat(sc.paramsJson, "aggroRange", 12.f);
    combat.enemyAttackRange = params.readScriptParamFloat(sc.paramsJson, "enemyAttackRange", 1.5f);
    enemiesPerWave = params.readScriptParamInt(sc.paramsJson, "enemiesPerWave", 5);
    spawnRadius = params.readScriptParamFloat(sc.paramsJson, "spawnRadius", 6.f);
    timeBetweenWaves = params.readScriptParamFloat(sc.paramsJson, "timeBetweenWaves", 3.f);
    combat.enemiesPerWave = enemiesPerWave;
    combat.spawnRadius = spawnRadius;
    combat.timeBetweenWaves = timeBetweenWaves;
  }

  void combatDirector::onCreate(Nebula::ScriptContext &ctx, Nebula::Entity self)
  {

    if (!ctx.scene.isValidEntity(self))
    {
      return;
    }
    const Nebula::ScriptComponent &sc = ctx.scene.getScriptComponent(self);
    syncCombatFromParams(sc, m_params, Nimbus::Combat::instance(), m_enemiesPerWave, m_spawnRadius, m_timeBetweenWaves);
  }

  void combatDirector::onEnable(Nebula::ScriptContext &ctx, Nebula::Entity self)
  {
    onCreate(ctx, self);
  }

  void combatDirector::onUpdate(Nebula::ScriptContext &ctx, Nebula::Entity self, float)
  {
    if (!ctx.scene.isValidEntity(self))
    {
      return;
    }
    const Nebula::ScriptComponent &sc = ctx.scene.getScriptComponent(self);
    syncCombatFromParams(sc, m_params, Nimbus::Combat::instance(), m_enemiesPerWave, m_spawnRadius, m_timeBetweenWaves);
  }
}
