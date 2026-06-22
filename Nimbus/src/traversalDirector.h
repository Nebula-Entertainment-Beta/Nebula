
#pragma once
#include "script.h"
#include "traversal.h"
#include "scriptParams.h"

namespace Nimbus
{
  class TraversalDirector : public Nebula::IScript
  {
  public:
    void onCreate(Nebula::ScriptContext &, Nebula::Entity) override;
    void onEnable(Nebula::ScriptContext &, Nebula::Entity) override;
    void onUpdate(Nebula::ScriptContext &, Nebula::Entity, float) override;

    const TraversalSettings &settings() const { return m_settings; }

  private:
    TraversalSettings m_settings{};
    Nebula::ScriptParams m_params;
  };
};