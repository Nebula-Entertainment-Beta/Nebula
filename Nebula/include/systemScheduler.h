#pragma once
#include <functional>
#include <vector>

namespace Nebula
{
  enum class SystemPhase
  {
    PreUpdate,
    Update,
    FixedUpdate,
    PostUpdate,
    Render
  };

  class SystemScheduler
  {
  public:
    using SystemFn = std::function<void(float dt)>;

    void add(SystemPhase phase, SystemFn fn);
    void run(SystemPhase phase, float dt);
    void runFixed(SystemPhase phase, float fixedDt, float frameDt,
                  float fixedStep = 1.f / 60.f, int maxSteps = 5);
    void resetFixedAccumulator() { m_fixedAccumulator = 0.f; }

  private:
    std::vector<SystemFn> m_preUpdate;
    std::vector<SystemFn> m_update;
    std::vector<SystemFn> m_fixedUpdate;
    std::vector<SystemFn> m_postUpdate;
    std::vector<SystemFn> m_render;
    float m_fixedAccumulator = 0.f;
    std::vector<SystemFn> &bucket(SystemPhase p);
  };
}