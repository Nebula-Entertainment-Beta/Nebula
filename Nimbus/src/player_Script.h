#pragma once
#include <script.h>

namespace Nimbus
{

  class PlayerScript : public Nebula::IScript
  {
  public:
    void onCreate(Nebula::ScriptContext &ctx, Nebula::Entity self) override;
    void onEnable(Nebula::ScriptContext &, Nebula::Entity) override {}
    void onUpdate(Nebula::ScriptContext &, Nebula::Entity, float) override;
    void onPhysicsUpdate(Nebula::ScriptContext &, Nebula::Entity, float) override {}
    void onRender(Nebula::ScriptContext &, Nebula::Entity, float) override {}
    void onDisable(Nebula::ScriptContext &, Nebula::Entity) override {}
    void onDestroy(Nebula::ScriptContext &, Nebula::Entity) override {}
    Nebula::Entity GetCamera(Nebula::ScriptContext &ctx, Nebula::Entity self);

  private:
    float m_moveSpeed = 3.f;
  };

}
