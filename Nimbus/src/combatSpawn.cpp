#include "combatSpawn.h"

namespace Nimbus
{

  Nebula::Entity spawnEnemy(Nebula::ISceneAccess &scene, const Nebula::Vec3 &position)
  {
    Nebula::Entity enemy = scene.createEntity();
    scene.setEntityTag(enemy, "Enemy");
    scene.setEntityTransform(enemy, position, 0.f, 1.f);
    scene.setEntityMeshRenderer(enemy, "builtin/meshes/cube", "builtin/materials/enemy");
    scene.setEntityScript(enemy, "Enemy");
    return enemy;
  }

}
