#pragma once
#include <Nebula.h>

namespace Nimbus
{
  bool isInsideSphere(Nebula::Vec3 point, Nebula::Vec3 center, float radius);
  std::vector<Nebula::Entity> findEnemiesInSphere(
      Nebula::ISceneAccess &scene, Nebula::Vec3 center, float radius);
}