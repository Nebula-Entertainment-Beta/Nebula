#include "physics/iphysics_world.h"
#include "component.h"
#include "scene.h"

namespace Nebula
{

  namespace
  {

    class NullPhysicsWorld final : public IPhysicsWorld
    {
    public:
      void step(float) override {}

      void syncTransformsToScene(Scene &) override {}

      void setBodyPosition(Scene &scene, Entity entity, const Vec3 &position) override
      {
        if (!scene.isValidEntity(entity) || !scene.hasComponent<TransformComponent>(entity))
        {
          return;
        }
        scene.getComponent<TransformComponent>(entity).transform.setPosition(position);
      }

      Vec3 getBodyPosition(const Scene &scene, Entity entity) const override
      {
        if (!scene.isValidEntity(entity) || !scene.hasComponent<TransformComponent>(entity))
        {
          return {};
        }
        return scene.getComponent<TransformComponent>(entity).transform.getPosition();
      }

      bool raycast(const Scene &, const Vec3 &, const Vec3 &, float, RaycastHit &) const override
      {
        return false;
      }
    };

  } // namespace

  std::unique_ptr<IPhysicsWorld> createNullPhysicsWorld()
  {
    return std::make_unique<NullPhysicsWorld>();
  }

} // namespace Nebula
