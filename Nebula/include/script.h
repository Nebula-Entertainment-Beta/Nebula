/**
 * @file script.h
 * @brief Script **lifecycle** interface and per-frame context passed to gameplay hooks.
 *
 * **Owns:** `ScriptContext` (`ISceneAccess` + optional `IInputQuery`), `IScript`, `ScriptPtr`.
 *
 * **Why:** Lets entities in JSON name a behavior (`ScriptComponent::scriptName`) while C++ classes
 * implement `onUpdate` / `onRender` / … without the engine depending on every game type.
 */
#pragma once

#include <memory>
#include "scene.h"
#include "inputQuery.h"
#include "logSink.h"
#include "sceneAccess.h"

namespace Nebula
{

  class IPhysicsQuery;
  class Scene;
  class AssetManager;
  class IAssetProvider;
  class IRenderResourceFactory;
  class IAudioService;

  struct ScriptContext
  {
    ISceneAccess &scene;
    IInputQuery *input = nullptr;   // nullptr = transform-only scripts
    ILogSink *log = nullptr;        // nullptr = no host console (standalone game)
    IPhysicsQuery *physics = nullptr;
    Scene *physicsScene = nullptr;
    AssetManager *assetManager = nullptr;
    const IAssetProvider *assets = nullptr;
    IRenderResourceFactory *renderResources = nullptr;
    IAudioService *audio = nullptr;
    void *scriptRebuildUserData = nullptr;
    void (*requestScriptRebuildFn)(void *userData) = nullptr;

    void requestScriptRebuild() const
    {
      if (requestScriptRebuildFn != nullptr)
      {
        requestScriptRebuildFn(scriptRebuildUserData);
      }
    }
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