#pragma once
#include <script.h>

namespace Nimbus
{

  class MainCameraScript : public Nebula::IScript
  {
  public:
    void onCreate(Nebula::ScriptContext &ctx, Nebula::Entity self) override;
    void onEnable(Nebula::ScriptContext &, Nebula::Entity) override {}
    void onUpdate(Nebula::ScriptContext &, Nebula::Entity, float) override;
    void onPhysicsUpdate(Nebula::ScriptContext &, Nebula::Entity, float) override {}
    void onRender(Nebula::ScriptContext &, Nebula::Entity, float) override {}
    void onDisable(Nebula::ScriptContext &, Nebula::Entity) override {}
    void onDestroy(Nebula::ScriptContext &, Nebula::Entity) override {}

  private:
    float m_lookSensitivity = 0.0035f;
    float m_zoomSpeed = 0.6f;
    float m_pitchMin = -1.2f;
    float m_pitchMax = 0.65f;
    float m_minDistance = 1.5f;
    float m_maxDistance = 24.f;
  };

}
