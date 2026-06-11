#include "waveSpawner.h"

#include "combatSpawn.h"

#include <chrono>
#include <cmath>
#include <fstream>

namespace Nimbus
{
  namespace
  {
    // #region agent log
    void agentLog(const char *location, const char *message, const char *hypothesisId, int count, bool spawned)
    {
      std::ofstream log("/Users/ibrahimfaisal/Nebula/.cursor/debug-c24e3e.log", std::ios::app);
      if (!log)
      {
        return;
      }
      const auto ts = std::chrono::duration_cast<std::chrono::milliseconds>(
                          std::chrono::system_clock::now().time_since_epoch())
                          .count();
      log << "{\"sessionId\":\"c24e3e\",\"hypothesisId\":\"" << hypothesisId
          << "\",\"location\":\"" << location << "\",\"message\":\"" << message
          << "\",\"data\":{\"count\":" << count << ",\"m_spawned\":" << (spawned ? 1 : 0)
          << "},\"timestamp\":" << ts << "}\n";
    }
    // #endregion
  }

  void WaveSpawner::onCreate(Nebula::ScriptContext &ctx, Nebula::Entity entity)
  {
    if (!ctx.scene.isValidEntity(entity))
    {
      return;
    }
    const Nebula::ScriptComponent &sc = ctx.scene.getScriptComponent(entity);
    m_waveEnemies = m_params.readScriptParamEntityVector(sc.paramsJson, "waveEnemies", {});
    if (!m_waveEnemies.empty())
    {
      m_spawned = true;
    }
    // #region agent log
    agentLog("waveSpawner.cpp:onCreate", "restored spawn state", "A", static_cast<int>(m_waveEnemies.size()),
             m_spawned);
    // #endregion
  }

  void WaveSpawner::onUpdate(Nebula::ScriptContext &ctx, Nebula::Entity entity, float)
  {
    if (m_spawned)
    {
      return;
    }

    spawnWave(ctx, entity);
    ctx.requestScriptRebuild();
    m_spawned = true;
    // #region agent log
    agentLog("waveSpawner.cpp:onUpdate", "spawned wave", "A", static_cast<int>(m_waveEnemies.size()), m_spawned);
    // #endregion
  }

  void WaveSpawner::spawnWave(Nebula::ScriptContext &ctx, Nebula::Entity entity)
  {
    const int count = Nimbus::Combat::instance().wavesPerSecond();
    const float radius = Nimbus::Combat::instance().spawnRadius;

    m_waveEnemies.clear();
    m_waveEnemies.reserve(static_cast<std::size_t>(count));

    for (int i = 0; i < count; ++i)
    {
      const float angle = (2.f * 3.14159265f * static_cast<float>(i)) / static_cast<float>(count);
      const float x = radius * std::cos(angle);
      const float z = radius * std::sin(angle);
      const Nebula::Vec3 position{x, 0.5f, z};
      m_waveEnemies.push_back(spawnEnemy(ctx.scene, position));
    }

    if (ctx.scene.isValidEntity(entity))
    {
      const Nebula::ScriptComponent &sc = ctx.scene.getScriptComponent(entity);
      const std::string paramsJson =
          m_params.setScriptParamEntityVector(sc.paramsJson, "waveEnemies", m_waveEnemies);
      ctx.scene.setScriptParamsJson(entity, paramsJson);
    }
  }

}
