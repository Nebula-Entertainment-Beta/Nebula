#pragma once
#include "script.h"
#include "scriptParams.h"

#include <string>

namespace Nimbus
{
  class TraversalDirector : public Nebula::IScript
  {
  public:
    void onCreate(Nebula::ScriptContext &, Nebula::Entity) override;
    void onEnable(Nebula::ScriptContext &, Nebula::Entity) override;
    void onUpdate(Nebula::ScriptContext &, Nebula::Entity, float) override;

  private:
    std::string m_lastParamsJson;
    Nebula::ScriptParams m_params;
  };
};
