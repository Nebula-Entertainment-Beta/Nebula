#include "physics_query_adapter.h"

#include "component.h"
#include "physics/collision_math.h"
#include "physics/physics_component.h"
#include "scene.h"

namespace Nebula
{

  namespace
  {
    constexpr float kGroundProbeDistance = 0.15f;
  }

  PhysicsQueryAdapter::PhysicsQueryAdapter(IPhysicsWorld &world) : m_world(&world) {}

  bool PhysicsQueryAdapter::raycast(const Scene &scene, const Vec3 &origin, const Vec3 &direction,
                                    float maxDistance, RaycastHit &out) const
  {
    return m_world != nullptr && m_world->raycast(scene, origin, direction, maxDistance, out);
  }

  bool PhysicsQueryAdapter::overlapBox(const Scene &scene, const Vec3 &center, const Vec3 &halfExtents,
                                       std::vector<OverlapHit> &out, OverlapFilter filter) const
  {
    return m_world != nullptr &&
           m_world->overlapBox(scene, center, halfExtents, out, filter);
  }

  void PhysicsQueryAdapter::moveKinematic(Scene &scene, Entity entity, const Vec3 &delta, bool &outGrounded)
  {
    if (m_world == nullptr)
    {
      outGrounded = false;
      return;
    }
    m_world->moveKinematic(scene, entity, delta, outGrounded);
  }

  bool PhysicsQueryAdapter::isGrounded(const Scene &scene, Entity entity) const
  {
    if (m_world == nullptr || !scene.isValidEntity(entity) ||
        !scene.hasComponent<TransformComponent>(entity) ||
        !scene.hasComponent<ColliderComponent>(entity))
    {
      return false;
    }

    Scene &mutableScene = const_cast<Scene &>(scene);
    const CollisionMath collisionMath;
    const AABB bounds = collisionMath.worldAABBFromEntity(mutableScene, entity);
    if (bounds.min.y >= bounds.max.y)
    {
      return false;
    }

    const Vec3 origin{
        (bounds.min.x + bounds.max.x) * 0.5f,
        bounds.min.y + 0.01f,
        (bounds.min.z + bounds.max.z) * 0.5f,
    };
    const Vec3 down{0.0f, -1.0f, 0.0f};
    RaycastHit hit{};
    if (!m_world->raycast(scene, origin, down, kGroundProbeDistance, hit))
    {
      return false;
    }
    return hit.entity != entity && hit.normal.y > 0.5f;
  }

} // namespace Nebula
