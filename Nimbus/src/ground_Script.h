#pragma once
#include <script.h>
class groundScript : public Nebula::IScript
{
public:
  void onCreate(Nebula::ScriptContext &, Nebula::Entity) override {}
  void onEnable(Nebula::ScriptContext &, Nebula::Entity) override {}
  void onUpdate(Nebula::ScriptContext &, Nebula::Entity, float) override {}
  void onPhysicsUpdate(Nebula::ScriptContext &, Nebula::Entity, float) override {}
  void onRender(Nebula::ScriptContext &, Nebula::Entity, float) override {}
  void onDisable(Nebula::ScriptContext &, Nebula::Entity) override {}
  void onDestroy(Nebula::ScriptContext &, Nebula::Entity) override {}
};