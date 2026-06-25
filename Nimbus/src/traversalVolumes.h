#pragma once

#include "ecs/entity.h"
#include "math_types.h"
#include "physicsQuery.h"
#include "scene.h"

#include <vector>

namespace Nimbus
{

  struct VolumeQueryContext
  {
    Nebula::IPhysicsQuery *physics = nullptr;
    Nebula::Scene *scene = nullptr;
  };

  bool getTriggerBoxQuery(const Nebula::Scene &scene, Nebula::Entity volume,
                          Nebula::Vec3 &outCenter, Nebula::Vec3 &outHalfExtents);

  bool isEntityOverlappingVolume(const VolumeQueryContext &ctx,
                                 Nebula::Entity volume,
                                 Nebula::Entity target);

  float queryBounceImpulseIfOverlapping(const VolumeQueryContext &ctx,
                                        Nebula::Entity player,
                                        float currentVelocityY);

  float queryWindLiftIfOverlapping(const VolumeQueryContext &ctx,
                                   Nebula::Entity player);

}
