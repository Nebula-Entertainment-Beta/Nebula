#include "encounterDirector.h"
#include "encounterState.h"
#include "nimbus_config.h"
#include "combat.h"

namespace Nimbus
{
  void EncounterDirector::onCreate(Nebula::ScriptContext &ctx, Nebula::Entity)
  {
    const Nebula::Entity player = ctx.scene.findByTag(kPlayerTag);
    if (ctx.scene.isValidEntity(player))
    {
      m_initialSpawn = ctx.scene.getTransform(player).transform.getPosition();
    }
    EncounterState::instance().resetForPlay(m_initialSpawn);
  }

  void EncounterDirector::onUpdate(Nebula::ScriptContext &ctx, Nebula::Entity, float)
  {
    EncounterState &enc = EncounterState::instance();
    if (enc.retryRequested)
    {
      applyRetry(ctx);
    }
  }

  void EncounterDirector::applyRetry(Nebula::ScriptContext &ctx)
  {
    EncounterState &enc = EncounterState::instance();
    const Nebula::Entity player = ctx.scene.findByTag(kPlayerTag);
    if (ctx.scene.isValidEntity(player))
    {
      const Nebula::Vec3 respawn =
          enc.hasCheckpoint ? enc.checkpointPosition : m_initialSpawn;
      ctx.scene.getTransform(player).transform.setPosition(respawn);
    }
    Combat::instance().pendingPlayerDamage = 0.f;
    Combat::instance().playerIFrameTimer = 0.f;
    Combat::instance().enemyHitRequests.clear();
    if (ctx.log != nullptr)
    {
      ctx.log->info("[Encounter] Retry — restored checkpoint");
    }
  }
}
