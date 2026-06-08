#include "combatHelper.h"

namespace Nimbus
{
  bool isInsideSphere(Nebula::Vec3 point, Nebula::Vec3 center, float radius)
  {
    float dx = point.x - center.x;
    float dy = point.y - center.y;
    float dz = point.z - center.z;
    float distanceSquared = dx * dx + dy * dy + dz * dz;
    return distanceSquared <= radius * radius;
  }

  std::vector<Nebula::Entity> findEnemiesInSphere(
      Nebula::ISceneAccess &scene, Nebula::Vec3 center, float radius)
  {
    std::vector<Nebula::Entity> hits;
    for (Nebula::Entity enemy : scene.findAllByTag("Enemy"))
    {
      if (!scene.isValidEntity(enemy))
      {
        continue;
      }
      const Nebula::Vec3 pos = scene.getTransform(enemy).transform.getPosition();
      if (isInsideSphere(pos, center, radius))
      {
        hits.push_back(enemy);
      }
    }
    return hits;
  }
}