#include "systemScheduler.h"

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

    static float accumulator = 0.f;
    accumulator += frameDt;
    int steps = 0;
    while (accumulator >= fixedStep && steps < maxSteps)
    {
      run(phase, fixedDt);
      accumulator -= fixedStep;
      steps++;
    }
    // Always simulate at least once per rendered frame so one-frame inputs (jump)
    // aren't dropped when frameDt < fixedStep.
    if (steps == 0 && frameDt > 0.f)
    {
      run(phase, fixedDt);
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