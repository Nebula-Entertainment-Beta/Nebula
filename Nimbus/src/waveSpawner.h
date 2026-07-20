#pragma once

#include <script.h>
#include <scriptParams.h>
#include <vector>

#include "combat.h"

namespace Nimbus
{

  class WaveSpawner : public Nebula::IScript
  {
  public:
    void onCreate(Nebula::ScriptContext &, Nebula::Entity) override;
    void onEnable(Nebula::ScriptContext &, Nebula::Entity) override {}
    void onUpdate(Nebula::ScriptContext &, Nebula::Entity, float) override;
    void onPhysicsUpdate(Nebula::ScriptContext &, Nebula::Entity, float) override {}
    void onRender(Nebula::ScriptContext &, Nebula::Entity, float) override {}
    void onDisable(Nebula::ScriptContext &, Nebula::Entity) override {}
    void onDestroy(Nebula::ScriptContext &, Nebula::Entity) override {}

  private:
    void spawnWave(Nebula::ScriptContext &, Nebula::Entity);
    void pruneDead(Nebula::ScriptContext &);
    void clearWave(Nebula::ScriptContext &);
    void writeProgress(Nebula::ScriptContext &, Nebula::Entity);

    bool m_spawned = false;
    float m_waitTimer = 0.f;
    int m_waveIndex = 0;
    int m_maxWaves = 3;
    Nebula::ScriptParams m_params;
    std::vector<Nebula::Entity> m_waveEnemies;
  };

}
