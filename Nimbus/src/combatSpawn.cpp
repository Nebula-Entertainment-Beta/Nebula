#include "combatSpawn.h"

#include "prefabService.h"

namespace Nimbus
{

  Nebula::Entity spawnEnemy(Nebula::ScriptContext &ctx, const Nebula::Vec3 &position)
  {
    if (ctx.assetManager != nullptr && ctx.assets != nullptr && ctx.renderResources != nullptr &&
        ctx.physicsScene != nullptr)
    {
      Nebula::Entity enemy = Nebula::PrefabService::instantiate(
          *ctx.physicsScene, *ctx.assetManager, *ctx.assets, *ctx.renderResources, "prefabs/enemy.prefab");
      if (enemy.id != 0 && ctx.scene.isValidEntity(enemy))
      {
        ctx.scene.setEntityTransform(enemy, position, 0.f, 1.f);
        return enemy;
      }
    }

    Nebula::Entity enemy = ctx.scene.createEntity();
    ctx.scene.setEntityTag(enemy, "Enemy");
    ctx.scene.setEntityTransform(enemy, position, 0.f, 1.f);
    ctx.scene.setEntityMeshRenderer(enemy, "builtin/meshes/cube", "builtin/materials/enemy");
    ctx.scene.setEntityScript(enemy, "Enemy");
    return enemy;
  }

}
