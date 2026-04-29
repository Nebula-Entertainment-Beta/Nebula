#pragma once

#include <memory>
#include "scene.h"

namespace Nebula
{
  class Input;
  class ActionMapping;

  struct ScriptContext
  {

    Scene &scene;
    Input *input = nullptr;
    ActionMapping *actions = nullptr;
  };

  class IScript
  {
  public:
    virtual ~IScript() = default;
    virtual void onCreate(ScriptContext &ctx, Entity self) {}
    virtual void onEnable(ScriptContext &ctx, Entity self) {}
    virtual void onUpdate(ScriptContext &ctx, Entity self, float dt) {}
    virtual void onPhysicsUpdate(ScriptContext &ctx, Entity self, float fixedDt) {}
    virtual void onRender(ScriptContext &ctx, Entity self, float dt) {}
    virtual void onDisable(ScriptContext &ctx, Entity self) {}
    virtual void onDestroy(ScriptContext &ctx, Entity self) {}
  };

  using ScriptPtr = std::unique_ptr<IScript>;

}