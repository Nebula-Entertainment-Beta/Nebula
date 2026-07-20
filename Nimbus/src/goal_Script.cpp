#include "goal_Script.h"
#include "encounterState.h"
#include "nimbus_config.h"
#include "traversalVolumes.h"
#include "audioService.h"
#include "nodeGraph.h"

namespace Nimbus
{
  void GoalScript::onCreate(Nebula::ScriptContext &, Nebula::Entity)
  {
    m_triggered = false;
  }

  void GoalScript::onUpdate(Nebula::ScriptContext &ctx, Nebula::Entity self, float)
  {
    EncounterState &enc = EncounterState::instance();
    if (m_triggered || enc.objectiveComplete || !ctx.scene.isValidEntity(self))
    {
      return;
    }
    if (enc.waveIndex < enc.wavesToWin)
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
    m_triggered = true;
    enc.objectiveComplete = true;
    if (ctx.audio != nullptr)
    {
      ctx.audio->playOneShot("audio/objective.wav", 1.f);
    }
    if (ctx.assets != nullptr)
    {
      Nebula::NodeGraphRuntime graph;
      if (graph.loadFromFile(*ctx.assets, "graphs/objective_complete.json"))
      {
        graph.fireEvent(ctx, "OnObjectiveReady");
      }
    }
    if (ctx.log != nullptr)
    {
      ctx.log->info("[Objective] Goal reached — vertical slice complete");
    }
  }
}
