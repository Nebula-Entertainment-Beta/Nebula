#include "systemScheduler.h"

#include <algorithm>

namespace Nebula
{
  void SystemScheduler::add(SystemPhase phase, SystemFn fn)
  {
    if (fn)
      bucket(phase).push_back(std::move(fn));
  }

  void SystemScheduler::run(SystemPhase phase, float dt)
  {
    for (const auto &fn : bucket(phase))
      fn(dt);
  }

  void SystemScheduler::runFixed(SystemPhase phase, float fixedDt, float frameDt, float fixedStep, int maxSteps)
  {
    // Cap catch-up so one hitch cannot schedule a multi-step death spiral.
    const float maxCatchUp = fixedStep * static_cast<float>(std::max(maxSteps, 1));
    m_fixedAccumulator += std::min(frameDt, maxCatchUp);
    int steps = 0;
    while (m_fixedAccumulator >= fixedStep && steps < maxSteps)
    {
      run(phase, fixedDt);
      m_fixedAccumulator -= fixedStep;
      steps++;
    }
  }

  std::vector<SystemScheduler::SystemFn> &SystemScheduler::bucket(SystemPhase p)
  {
    switch (p)
    {
    case SystemPhase::PreUpdate:
      return m_preUpdate;
    case SystemPhase::Update:
      return m_update;
    case SystemPhase::FixedUpdate:
      return m_fixedUpdate;
    case SystemPhase::PostUpdate:
      return m_postUpdate;
    case SystemPhase::Render:
      return m_render;
    }
    return m_update;
  }

}