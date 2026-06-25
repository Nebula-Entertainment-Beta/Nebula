#include "windVolume_Script.h"

#include "component.h"
#include "physics/physics_component.h"

namespace Nimbus
{

  void WindVolumeScript::onCreate(Nebula::ScriptContext &ctx, Nebula::Entity self)
  {
    if (!ctx.scene.isValidEntity(self) || ctx.log == nullptr)
    {
      return;
    }

    if (ctx.physicsScene == nullptr ||
        !ctx.physicsScene->hasComponent<Nebula::ColliderComponent>(self))
    {
      ctx.log->info("[WindVolume] Missing ColliderComponent on entity id=" +
                    std::to_string(self.id));
      return;
    }

    const auto &collider = ctx.physicsScene->getComponent<Nebula::ColliderComponent>(self);
    if (!collider.isTrigger)
    {
      ctx.log->info("[WindVolume] Collider should be a trigger on entity id=" +
                    std::to_string(self.id));
    }
  }

}
