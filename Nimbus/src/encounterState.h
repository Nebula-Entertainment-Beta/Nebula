#pragma once

#include "ecs/entity.h"
#include "math_types.h"

namespace Nimbus
{

  /** Shared encounter/runtime state for waves, checkpoints, fail/retry, and goals. */
  struct EncounterState
  {
    Nebula::Vec3 checkpointPosition{0.f, 0.5f, 0.f};
    bool hasCheckpoint = false;
    bool objectiveComplete = false;
    bool retryRequested = false;
    bool failLatched = false;
    bool restorePlayer = false;
    int waveIndex = 0;
    int wavesToWin = 3;

    static EncounterState &instance()
    {
      static EncounterState s;
      return s;
    }

    void resetForPlay(const Nebula::Vec3 &spawn)
    {
      checkpointPosition = spawn;
      hasCheckpoint = true;
      objectiveComplete = false;
      retryRequested = false;
      failLatched = false;
      restorePlayer = false;
      waveIndex = 0;
    }

    void requestRetry()
    {
      if (!objectiveComplete)
      {
        retryRequested = true;
        failLatched = true;
        restorePlayer = true;
      }
    }
  };

}
