#pragma once
#include <script.h>
#include <scriptParams.h>
#include "math_types.h"

namespace Nimbus
{
  class EncounterDirector : public Nebula::IScript
  {
  public:
    void onCreate(Nebula::ScriptContext &, Nebula::Entity) override;
    void onUpdate(Nebula::ScriptContext &, Nebula::Entity, float) override;
    void onPhysicsUpdate(Nebula::ScriptContext &, Nebula::Entity, float) override {}
    void onRender(Nebula::ScriptContext &, Nebula::Entity, float) override {}
    void onEnable(Nebula::ScriptContext &, Nebula::Entity) override {}
    void onDisable(Nebula::ScriptContext &, Nebula::Entity) override {}
    void onDestroy(Nebula::ScriptContext &, Nebula::Entity) override {}

  private:
    void applyRetry(Nebula::ScriptContext &ctx);
    Nebula::Vec3 m_initialSpawn{0.f, 0.5f, 0.f};
    Nebula::ScriptParams m_params;
  };
}
