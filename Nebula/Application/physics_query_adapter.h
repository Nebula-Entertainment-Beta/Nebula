#pragma once

#include "physics/iphysics_world.h"
#include "physicsQuery.h"

namespace Nebula
{

  class PhysicsQueryAdapter final : public IPhysicsQuery
  {
  public:
    explicit PhysicsQueryAdapter(IPhysicsWorld &world);

    bool raycast(const Scene &scene, const Vec3 &origin, const Vec3 &direction, float maxDistance,
                 RaycastHit &out) const override;

    bool overlapBox(const Scene &scene, const Vec3 &center, const Vec3 &halfExtents,
                    std::vector<OverlapHit> &out,
                    OverlapFilter filter = OverlapFilter::All) const override;

    void moveKinematic(Scene &scene, Entity entity, const Vec3 &delta, bool &outGrounded) override;

    bool isGrounded(const Scene &scene, Entity entity) const override;

  private:
    IPhysicsWorld *m_world = nullptr;
  };

} // namespace Nebula
