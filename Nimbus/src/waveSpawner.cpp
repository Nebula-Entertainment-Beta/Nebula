#include "waveSpawner.h"

#include "combatSpawn.h"
#include "encounterState.h"
#include "nimbus_config.h"
#include "tag_component.h"

#include <cmath>
#include <string>

namespace Nimbus
{

  void WaveSpawner::onCreate(Nebula::ScriptContext &ctx, Nebula::Entity entity)
  {
    if (!ctx.scene.isValidEntity(entity))
    {
      return;
    }
    const Nebula::ScriptComponent &sc = ctx.scene.getScriptComponent(entity);
    m_waveEnemies = m_params.readScriptParamEntityVector(sc.paramsJson, "waveEnemies", {});
    m_waveIndex = m_params.readScriptParamInt(sc.paramsJson, "waveIndex", 0);
    m_maxWaves = m_params.readScriptParamInt(sc.paramsJson, "maxWaves", 3);
    m_spawned = !m_waveEnemies.empty();
    m_waitTimer = 0.f;
    EncounterState::instance().waveIndex = m_waveIndex;
    EncounterState::instance().wavesToWin = m_maxWaves;
  }

  void WaveSpawner::onUpdate(Nebula::ScriptContext &ctx, Nebula::Entity entity, float dt)
  {
    EncounterState &enc = EncounterState::instance();
    if (enc.objectiveComplete)
    {
      return;
    }

    if (enc.retryRequested)
    {
      clearWave(ctx);
      m_spawned = false;
      m_waitTimer = 0.f;
      m_waveIndex = 0;
      enc.waveIndex = 0;
      enc.retryRequested = false;
      enc.failLatched = false;
      writeProgress(ctx, entity);
      return;
    }

    pruneDead(ctx);

    if (!m_spawned)
    {
      if (m_waveIndex >= m_maxWaves)
      {
        return;
      }
      spawnWave(ctx, entity);
      ctx.requestScriptRebuild();
      m_spawned = true;
      m_waitTimer = 0.f;
      writeProgress(ctx, entity);
      if (ctx.log != nullptr)
      {
        ctx.log->info("[WaveSpawner] Spawned wave " + std::to_string(m_waveIndex + 1) + "/" +
                      std::to_string(m_maxWaves));
      }
      return;
    }

    if (!m_waveEnemies.empty())
    {
      return;
    }

    // Wave cleared.
    m_waveIndex++;
    enc.waveIndex = m_waveIndex;
    writeProgress(ctx, entity);
    if (m_waveIndex >= m_maxWaves)
    {
      if (ctx.log != nullptr)
      {
        ctx.log->info("[WaveSpawner] All waves cleared");
      }
      return;
    }

    m_waitTimer += dt;
    const float delay = Nimbus::Combat::instance().timeBetweenWaves;
    if (m_waitTimer >= delay)
    {
      m_spawned = false;
      m_waitTimer = 0.f;
    }
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
      m_waveEnemies.push_back(spawnEnemy(ctx, position));
    }

    writeProgress(ctx, entity);
  }

  void WaveSpawner::pruneDead(Nebula::ScriptContext &ctx)
  {
    std::vector<Nebula::Entity> alive;
    alive.reserve(m_waveEnemies.size());
    for (const Nebula::Entity e : m_waveEnemies)
    {
      if (!ctx.scene.isValidEntity(e))
      {
        continue;
      }
      const Nebula::Vec3 scale = ctx.scene.getTransform(e).transform.getScale();
      // Enemy death shrinks scale to zero; treat near-zero as dead.
      if (scale.x <= 0.01f && scale.y <= 0.01f && scale.z <= 0.01f)
      {
        continue;
      }
      alive.push_back(e);
    }
    m_waveEnemies.swap(alive);
  }

  void WaveSpawner::clearWave(Nebula::ScriptContext &ctx)
  {
    for (const Nebula::Entity e : m_waveEnemies)
    {
      if (ctx.physicsScene != nullptr && ctx.physicsScene->isValidEntity(e))
      {
        ctx.physicsScene->destroyEntity(e);
      }
      else if (ctx.scene.isValidEntity(e))
      {
        ctx.scene.getTransform(e).transform.setScale({0.f, 0.f, 0.f});
      }
    }
    m_waveEnemies.clear();
  }

  void WaveSpawner::writeProgress(Nebula::ScriptContext &ctx, Nebula::Entity entity)
  {
    if (!ctx.scene.isValidEntity(entity))
    {
      return;
    }
    const Nebula::ScriptComponent &sc = ctx.scene.getScriptComponent(entity);
    std::string paramsJson =
        m_params.setScriptParamEntityVector(sc.paramsJson, "waveEnemies", m_waveEnemies);
    paramsJson = m_params.setScriptParamInt(paramsJson, "waveIndex", m_waveIndex);
    paramsJson = m_params.setScriptParamInt(paramsJson, "maxWaves", m_maxWaves);
    ctx.scene.setScriptParamsJson(entity, paramsJson);
  }

}
