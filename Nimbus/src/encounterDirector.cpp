#include "encounterDirector.h"
#include "nimbusRuntime.h"
#include "nimbus_config.h"

namespace Nimbus
{
  void EncounterDirector::onCreate(Nebula::ScriptContext &ctx, Nebula::Entity)
  {
    const Nebula::Entity player = ctx.scene.findByTag(kPlayerTag);
    if (ctx.scene.isValidEntity(player))
    {
      m_initialSpawn = ctx.scene.getTransform(player).transform.getPosition();
    }
    encounter(ctx).resetForPlay(m_initialSpawn);
    combat(ctx).clearTransient();
  }

  void EncounterDirector::onUpdate(Nebula::ScriptContext &ctx, Nebula::Entity, float)
  {
    if (encounter(ctx).retryRequested)
    {
      applyRetry(ctx);
    }
  }

  void EncounterDirector::applyRetry(Nebula::ScriptContext &ctx)
  {
    EncounterState &enc = encounter(ctx);
    const Nebula::Entity player = ctx.scene.findByTag(kPlayerTag);
    if (ctx.scene.isValidEntity(player))
    {
      const Nebula::Vec3 respawn =
          enc.hasCheckpoint ? enc.checkpointPosition : m_initialSpawn;
      ctx.scene.getTransform(player).transform.setPosition(respawn);
    }
    combat(ctx).clearTransient();
    if (ctx.log != nullptr)
    {
      ctx.log->info("[Encounter] Retry — restored checkpoint");
    }
  }
}
