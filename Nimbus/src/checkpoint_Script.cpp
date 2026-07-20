#include "checkpoint_Script.h"
#include "encounterState.h"
#include "nimbus_config.h"
#include "traversalVolumes.h"

namespace Nimbus
{
  void CheckpointScript::onCreate(Nebula::ScriptContext &, Nebula::Entity)
  {
    m_armed = true;
  }

  void CheckpointScript::onUpdate(Nebula::ScriptContext &ctx, Nebula::Entity self, float)
  {
    if (!m_armed || !ctx.scene.isValidEntity(self))
    {
      return;
    }
    const Nebula::Entity player = ctx.scene.findByTag(kPlayerTag);
    if (!ctx.scene.isValidEntity(player) || ctx.physics == nullptr || ctx.physicsScene == nullptr)
    {
      return;
    }
    VolumeQueryContext vctx{ctx.physics, ctx.physicsScene};
    if (!isEntityOverlappingVolume(vctx, self, player))
    {
      return;
    }
    const Nebula::Vec3 pos = ctx.scene.getTransform(self).transform.getPosition();
    EncounterState::instance().checkpointPosition = pos;
    EncounterState::instance().hasCheckpoint = true;
    m_armed = false;
    if (ctx.log != nullptr)
    {
      ctx.log->info("[Checkpoint] Activated");
    }
  }
}
