#pragma once
#include <script.h>
#include "combat.h"
#include "scriptParams.h"

namespace Nimbus
{

  class combatDirector : public Nebula::IScript
  {
  public:
    void onCreate(Nebula::ScriptContext &, Nebula::Entity) override;
    void onEnable(Nebula::ScriptContext &, Nebula::Entity) override;
    void onUpdate(Nebula::ScriptContext &, Nebula::Entity, float) override;
    void onPhysicsUpdate(Nebula::ScriptContext &, Nebula::Entity, float) override {}
    void onRender(Nebula::ScriptContext &, Nebula::Entity, float) override {}
    void onDisable(Nebula::ScriptContext &, Nebula::Entity) override {}
    void onDestroy(Nebula::ScriptContext &, Nebula::Entity) override {}

  private:
    int m_enemiesPerWave;
    float m_spawnRadius;
    float m_timeBetweenWaves;
    Nebula::ScriptParams m_params;
  };
}
