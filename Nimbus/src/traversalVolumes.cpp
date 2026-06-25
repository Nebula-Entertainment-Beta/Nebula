#include "traversalVolumes.h"

#include "component.h"
#include "nimbus_config.h"
#include "physics/collision_math.h"
#include "physics/physics_component.h"
#include "scene_query.h"
#include "scriptParams.h"

#include <algorithm>

namespace Nimbus
{

  bool getTriggerBoxQuery(const Nebula::Scene &scene, Nebula::Entity volume,
                          Nebula::Vec3 &outCenter, Nebula::Vec3 &outHalfExtents)
  {
    if (!scene.isValidEntity(volume) ||
        !scene.hasComponent<Nebula::TransformComponent>(volume) ||
        !scene.hasComponent<Nebula::ColliderComponent>(volume))
    {
      return false;
    }

    Nebula::Scene &mutableScene = const_cast<Nebula::Scene &>(scene);
    const Nebula::CollisionMath collisionMath;
    const Nebula::AABB bounds = collisionMath.worldAABBFromEntity(mutableScene, volume);
    if (bounds.min.y >= bounds.max.y)
    {
      return false;
    }

    outCenter = {
        (bounds.min.x + bounds.max.x) * 0.5f,
        (bounds.min.y + bounds.max.y) * 0.5f,
        (bounds.min.z + bounds.max.z) * 0.5f,
    };
    outHalfExtents = {
        (bounds.max.x - bounds.min.x) * 0.5f,
        (bounds.max.y - bounds.min.y) * 0.5f,
        (bounds.max.z - bounds.min.z) * 0.5f,
    };
    return true;
  }

  bool isEntityOverlappingVolume(const VolumeQueryContext &ctx,
                                 Nebula::Entity volume,
                                 Nebula::Entity target)
  {
    if (ctx.physics == nullptr || ctx.scene == nullptr ||
        !ctx.scene->isValidEntity(volume) || !ctx.scene->isValidEntity(target))
    {
      return false;
    }

    Nebula::Vec3 center{};
    Nebula::Vec3 half{};
    if (!getTriggerBoxQuery(*ctx.scene, volume, center, half))
    {
      return false;
    }

    std::vector<Nebula::OverlapHit> hits;
    ctx.physics->overlapBox(*ctx.scene, center, half, hits);
    for (const Nebula::OverlapHit &hit : hits)
    {
      if (hit.entity == target)
      {
        return true;
      }
    }
    return false;
  }

  float queryBounceImpulseIfOverlapping(const VolumeQueryContext &ctx,
                                        Nebula::Entity player,
                                        float currentVelocityY)
  {
    if (ctx.scene == nullptr || !ctx.scene->isValidEntity(player))
    {
      return 0.f;
    }

    Nebula::ScriptParams params;
    float best = 0.f;
    for (const Nebula::Entity pad : Nebula::findAllByTag(*ctx.scene, kBouncePadTag))
    {
      if (!isEntityOverlappingVolume(ctx, pad, player))
      {
        continue;
      }

      const Nebula::ScriptComponent &sc = ctx.scene->getComponent<Nebula::ScriptComponent>(pad);
      const float impulse = params.readScriptParamFloat(sc.paramsJson, "impulse", 14.f);
      if (currentVelocityY <= 0.f)
      {
        best = std::max(best, impulse);
      }
    }
    return best;
  }

  float queryWindLiftIfOverlapping(const VolumeQueryContext &ctx,
                                   Nebula::Entity player)
  {
    if (ctx.scene == nullptr || !ctx.scene->isValidEntity(player))
    {
      return 0.f;
    }

    Nebula::ScriptParams params;
    float best = 0.f;
    for (const Nebula::Entity volume : Nebula::findAllByTag(*ctx.scene, kWindVolumeTag))
    {
      if (!isEntityOverlappingVolume(ctx, volume, player))
      {
        continue;
      }

      const Nebula::ScriptComponent &sc = ctx.scene->getComponent<Nebula::ScriptComponent>(volume);
      const float liftSpeed = params.readScriptParamFloat(sc.paramsJson, "liftSpeed", 6.f);
      best = std::max(best, liftSpeed);
    }
    return best;
  }

}
