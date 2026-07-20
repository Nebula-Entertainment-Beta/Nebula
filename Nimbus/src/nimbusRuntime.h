#pragma once

#include "combat.h"
#include "encounterState.h"
#include "script.h"
#include "traversal.h"

#include <cassert>

namespace Nimbus
{

  /** Per-session gameplay state owned by the Nimbus host (game or editor). */
  struct NimbusRuntime
  {
    Combat combat;
    EncounterState encounter;
    TraversalSettings traversal;
  };

  inline NimbusRuntime &runtime(Nebula::ScriptContext &ctx)
  {
    assert(ctx.gameUserData != nullptr && "Nimbus scripts require a host-owned NimbusRuntime");
    return *static_cast<NimbusRuntime *>(ctx.gameUserData);
  }

  inline Combat &combat(Nebula::ScriptContext &ctx) { return runtime(ctx).combat; }

  inline EncounterState &encounter(Nebula::ScriptContext &ctx) { return runtime(ctx).encounter; }

  inline TraversalSettings &traversal(Nebula::ScriptContext &ctx) { return runtime(ctx).traversal; }

}
