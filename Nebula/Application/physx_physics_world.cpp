#include "physics/iphysics_world.h"

#include "component.h"
#include "physics/physics_component.h"
#include "scene.h"

#include <PxPhysicsAPI.h>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <memory>
#include <unordered_map>
#include <vector>

namespace Nebula
{
  namespace
  {
    using namespace physx;

    struct EntityKey
    {
      EntityID id = 0;
      uint32_t generation = 0;

      static EntityKey from(Entity e) { return {e.id, e.generation}; }
      Entity toEntity() const { return Entity{id, generation}; }

      bool operator==(const EntityKey &o) const { return id == o.id && generation == o.generation; }
    };

    struct EntityKeyHash
    {
      std::size_t operator()(const EntityKey &k) const noexcept
      {
        return std::hash<EntityID>{}(k.id) ^ (std::hash<uint32_t>{}(k.generation) << 1);
      }
    };

    PxVec3 toPx(const Vec3 &v) { return PxVec3(v.x, v.y, v.z); }
    Vec3 fromPx(const PxVec3 &v) { return Vec3{v.x, v.y, v.z}; }

    PxQuat yawToQuat(float yaw)
    {
      return PxQuat(yaw, PxVec3(0.f, 1.f, 0.f));
    }

    float quatToYaw(const PxQuat &q)
    {
      // Extract yaw about Y from quaternion (engine Transform3D is yaw-only).
      const float siny = 2.f * (q.w * q.y + q.x * q.z);
      const float cosy = 1.f - 2.f * (q.y * q.y + q.x * q.x);
      return std::atan2(siny, cosy);
    }

    bool passesFilter(const PxShape *shape, OverlapFilter filter)
    {
      if (shape == nullptr)
      {
        return false;
      }
      const bool trigger = shape->getFlags().isSet(PxShapeFlag::eTRIGGER_SHAPE);
      switch (filter)
      {
      case OverlapFilter::All:
        return true;
      case OverlapFilter::SolidOnly:
        return !trigger;
      case OverlapFilter::TriggersOnly:
        return trigger;
      }
      return true;
    }

    class PhysXWorld final : public IPhysicsWorld
    {
    public:
      PhysXWorld()
      {
        m_foundation = PxCreateFoundation(PX_PHYSICS_VERSION, m_allocator, m_errorCallback);
        if (m_foundation == nullptr)
        {
          return;
        }

        PxTolerancesScale scale;
        m_physics = PxCreatePhysics(PX_PHYSICS_VERSION, *m_foundation, scale, true, nullptr);
        if (m_physics == nullptr)
        {
          return;
        }

        PxSceneDesc desc(m_physics->getTolerancesScale());
        desc.gravity = PxVec3(0.f, -9.81f, 0.f);
        m_dispatcher = PxDefaultCpuDispatcherCreate(2);
        desc.cpuDispatcher = m_dispatcher;
        desc.filterShader = PxDefaultSimulationFilterShader;
        desc.flags |= PxSceneFlag::eENABLE_PCM;
        m_scene = m_physics->createScene(desc);
        m_material = m_physics->createMaterial(0.5f, 0.5f, 0.1f);
        m_valid = m_scene != nullptr && m_material != nullptr;
      }

      ~PhysXWorld() override
      {
        for (auto &entry : m_bodies)
        {
          if (entry.second.actor != nullptr)
          {
            entry.second.actor->release();
          }
        }
        m_bodies.clear();
        if (m_material != nullptr)
        {
          m_material->release();
        }
        if (m_scene != nullptr)
        {
          m_scene->release();
        }
        if (m_dispatcher != nullptr)
        {
          m_dispatcher->release();
        }
        if (m_physics != nullptr)
        {
          m_physics->release();
        }
        if (m_foundation != nullptr)
        {
          m_foundation->release();
        }
      }

      void step(Scene &scene, float fixedDt) override
      {
        if (!m_valid)
        {
          return;
        }
        reconcile(scene);
        m_scene->simulate(std::max(fixedDt, 1.f / 240.f));
        m_scene->fetchResults(true);
      }

      void syncTransformsToScene(Scene &scene) override
      {
        if (!m_valid)
        {
          return;
        }
        for (auto &[key, body] : m_bodies)
        {
          if (body.actor == nullptr || body.kinematic || body.isStatic)
          {
            continue;
          }
          const Entity entity = key.toEntity();
          if (!scene.isValidEntity(entity) || !scene.hasComponent<TransformComponent>(entity))
          {
            continue;
          }
          const PxTransform pose = body.actor->getGlobalPose();
          auto &tf = scene.getComponent<TransformComponent>(entity).transform;
          tf.setPosition(fromPx(pose.p));
          tf.setYaw(quatToYaw(pose.q));
        }
      }

      void setBodyPosition(Scene &scene, Entity entity, const Vec3 &position) override
      {
        if (scene.isValidEntity(entity) && scene.hasComponent<TransformComponent>(entity))
        {
          scene.getComponent<TransformComponent>(entity).transform.setPosition(position);
        }
        const EntityKey key = EntityKey::from(entity);
        auto it = m_bodies.find(key);
        if (it == m_bodies.end() || it->second.actor == nullptr)
        {
          return;
        }
        PxTransform pose = it->second.actor->getGlobalPose();
        pose.p = toPx(position);
        if (it->second.kinematic)
        {
          static_cast<PxRigidDynamic *>(it->second.actor)->setKinematicTarget(pose);
        }
        else
        {
          it->second.actor->setGlobalPose(pose);
        }
        it->second.commandedPose = pose;
      }

      Vec3 getBodyPosition(const Scene &scene, Entity entity) const override
      {
        const EntityKey key = EntityKey::from(entity);
        auto it = m_bodies.find(key);
        if (it != m_bodies.end() && it->second.actor != nullptr)
        {
          return fromPx(it->second.actor->getGlobalPose().p);
        }
        if (scene.isValidEntity(entity) && scene.hasComponent<TransformComponent>(entity))
        {
          return scene.getComponent<TransformComponent>(entity).transform.getPosition();
        }
        return {};
      }

      bool raycast(const Scene &, const Vec3 &origin, const Vec3 &direction, float maxDistance,
                   RaycastHit &out) const override
      {
        if (!m_valid)
        {
          return false;
        }
        PxVec3 dir = toPx(direction);
        const float len = dir.normalize();
        if (len <= 0.f)
        {
          return false;
        }
        PxRaycastBuffer hit;
        PxQueryFilterData filter;
        filter.flags = PxQueryFlag::eSTATIC | PxQueryFlag::eDYNAMIC | PxQueryFlag::ePREFILTER;
        QueryFilter callback(OverlapFilter::SolidOnly, nullptr);
        if (!m_scene->raycast(toPx(origin), dir, maxDistance, hit,
                              PxHitFlag::eDEFAULT | PxHitFlag::ePOSITION | PxHitFlag::eNORMAL,
                              filter, &callback) ||
            !hit.hasBlock)
        {
          return false;
        }
        const EntityKey *key = static_cast<const EntityKey *>(hit.block.actor->userData);
        if (key == nullptr)
        {
          return false;
        }
        out.entity = key->toEntity();
        out.distance = hit.block.distance;
        out.point = fromPx(hit.block.position);
        out.normal = fromPx(hit.block.normal);
        return true;
      }

      bool overlapSphere(const Scene &, const Vec3 &center, float radius, std::vector<OverlapHit> &out,
                         OverlapFilter filter) const override
      {
        return overlapGeometry(PxSphereGeometry(radius), PxTransform(toPx(center)), out, filter);
      }

      bool overlapBox(const Scene &, const Vec3 &center, const Vec3 &halfExtents,
                      std::vector<OverlapHit> &out, OverlapFilter filter) const override
      {
        return overlapGeometry(PxBoxGeometry(halfExtents.x, halfExtents.y, halfExtents.z),
                               PxTransform(toPx(center)), out, filter);
      }

      void moveKinematic(Scene &scene, Entity entity, const Vec3 &delta, bool &outGrounded) override
      {
        outGrounded = false;
        if (!m_valid)
        {
          return;
        }
        const EntityKey key = EntityKey::from(entity);
        auto it = m_bodies.find(key);
        if (it == m_bodies.end() || it->second.actor == nullptr)
        {
          reconcile(scene);
          it = m_bodies.find(key);
        }
        if (it == m_bodies.end() || it->second.actor == nullptr)
        {
          if (scene.isValidEntity(entity) && scene.hasComponent<TransformComponent>(entity))
          {
            auto &tf = scene.getComponent<TransformComponent>(entity).transform;
            const Vec3 p = tf.getPosition();
            tf.setPosition({p.x + delta.x, p.y + delta.y, p.z + delta.z});
          }
          return;
        }

        BodyRecord &body = it->second;
        PxTransform pose = body.commandedPose;
        PxVec3 remaining = toPx(delta);
        PxGeometryHolder geom = body.shape != nullptr ? body.shape->getGeometry() : PxGeometryHolder();

        constexpr int kMaxIter = 4;
        constexpr float kSkin = 0.02f;
        for (int i = 0; i < kMaxIter; ++i)
        {
          const float dist = remaining.magnitude();
          if (dist <= 1e-5f || body.shape == nullptr)
          {
            break;
          }
          PxVec3 dir = remaining.getNormalized();
          PxSweepBuffer hit;
          QueryFilter callback(OverlapFilter::SolidOnly, body.actor);
          PxQueryFilterData filter;
          filter.flags = PxQueryFlag::eSTATIC | PxQueryFlag::eDYNAMIC | PxQueryFlag::ePREFILTER;
          const bool blocked =
              m_scene->sweep(geom.any(), pose, dir, dist, hit,
                             PxHitFlag::eDEFAULT | PxHitFlag::eNORMAL | PxHitFlag::ePOSITION, filter,
                             &callback);
          if (!blocked || !hit.hasBlock)
          {
            pose.p += remaining;
            remaining = PxVec3(0.f);
            break;
          }
          const float travel = std::max(0.f, hit.block.distance - kSkin);
          pose.p += dir * travel;
          remaining -= dir * travel;
          if (hit.block.normal.y > 0.5f && dir.y < 0.f)
          {
            outGrounded = true;
          }
          const float vn = remaining.dot(hit.block.normal);
          if (vn < 0.f)
          {
            remaining -= hit.block.normal * vn;
          }
        }

        if (body.kinematic)
        {
          static_cast<PxRigidDynamic *>(body.actor)->setKinematicTarget(pose);
        }
        else
        {
          body.actor->setGlobalPose(pose);
        }
        body.commandedPose = pose;

        if (scene.isValidEntity(entity) && scene.hasComponent<TransformComponent>(entity))
        {
          auto &tf = scene.getComponent<TransformComponent>(entity).transform;
          tf.setPosition(fromPx(pose.p));
          tf.setYaw(quatToYaw(pose.q));
        }

        if (!outGrounded && delta.y <= 0.f && body.shape != nullptr)
        {
          PxSweepBuffer groundHit;
          QueryFilter callback(OverlapFilter::SolidOnly, body.actor);
          PxQueryFilterData filter;
          filter.flags = PxQueryFlag::eSTATIC | PxQueryFlag::eDYNAMIC | PxQueryFlag::ePREFILTER;
          if (m_scene->sweep(geom.any(), pose, PxVec3(0.f, -1.f, 0.f), 0.25f, groundHit,
                             PxHitFlag::eDEFAULT | PxHitFlag::eNORMAL, filter, &callback) &&
              groundHit.hasBlock && groundHit.block.normal.y > 0.5f)
          {
            outGrounded = true;
          }
        }
      }

    private:
      struct BodyRecord
      {
        std::unique_ptr<EntityKey> keyOwned;
        PxRigidActor *actor = nullptr;
        PxShape *shape = nullptr;
        PxTransform commandedPose{PxIdentity};
        bool isStatic = true;
        bool kinematic = false;
        ColliderComponent::Shape shapeKind = ColliderComponent::Shape::Box;
        Vec3 halfExtents{0.5f, 0.5f, 0.5f};
        Vec3 scale{1.f, 1.f, 1.f};
      };

      class QueryFilter : public PxQueryFilterCallback
      {
      public:
        QueryFilter(OverlapFilter filter, PxRigidActor *ignore)
            : m_filter(filter), m_ignore(ignore)
        {
        }

        PxQueryHitType::Enum preFilter(const PxFilterData &, const PxShape *shape,
                                       const PxRigidActor *actor, PxHitFlags &) override
        {
          if (actor == m_ignore || shape == nullptr || !passesFilter(shape, m_filter))
          {
            return PxQueryHitType::eNONE;
          }
          return PxQueryHitType::eBLOCK;
        }

        PxQueryHitType::Enum postFilter(const PxFilterData &, const PxQueryHit &, const PxShape *,
                                         const PxRigidActor *) override
        {
          return PxQueryHitType::eBLOCK;
        }

      private:
        OverlapFilter m_filter;
        PxRigidActor *m_ignore;
      };

      template <typename Geom>
      bool overlapGeometry(const Geom &geom, const PxTransform &pose, std::vector<OverlapHit> &out,
                           OverlapFilter filter) const
      {
        out.clear();
        if (!m_valid)
        {
          return false;
        }
        PxOverlapBufferN<128> hits;
        QueryFilter callback(filter, nullptr);
        PxQueryFilterData qfd;
        qfd.flags = PxQueryFlag::eSTATIC | PxQueryFlag::eDYNAMIC | PxQueryFlag::ePREFILTER |
                    PxQueryFlag::eNO_BLOCK;
        m_scene->overlap(geom, pose, hits, qfd, &callback);
        for (PxU32 i = 0; i < hits.getNbAnyHits(); ++i)
        {
          const PxOverlapHit &h = hits.getAnyHit(i);
          if (h.actor == nullptr || h.shape == nullptr)
          {
            continue;
          }
          const EntityKey *key = static_cast<const EntityKey *>(h.actor->userData);
          if (key == nullptr)
          {
            continue;
          }
          OverlapHit oh{};
          oh.entity = key->toEntity();
          PxVec3 dir{};
          PxF32 depth = 0.f;
          const PxGeometryHolder hitHolder = h.shape->getGeometry();
          const PxGeometry &hitGeom = hitHolder.any();
          const PxTransform hitPose = h.actor->getGlobalPose() * h.shape->getLocalPose();
          if (PxGeometryQuery::computePenetration(dir, depth, geom, pose, hitGeom, hitPose))
          {
            oh.normal = fromPx(dir);
            oh.penetration = depth;
          }
          out.push_back(oh);
        }
        return !out.empty();
      }

      void reconcile(Scene &scene)
      {
        std::unordered_map<EntityKey, bool, EntityKeyHash> seen;
        for (const Entity entity : scene.getAllEntities())
        {
          if (!scene.hasComponent<ColliderComponent>(entity) ||
              !scene.hasComponent<TransformComponent>(entity))
          {
            continue;
          }
          const EntityKey key = EntityKey::from(entity);
          seen[key] = true;
          const auto &collider = scene.getComponent<ColliderComponent>(entity);
          const auto &tf = scene.getComponent<TransformComponent>(entity).transform;
          const bool hasBody = scene.hasComponent<RigidBodyComponent>(entity);
          const bool kinematic =
              hasBody ? scene.getComponent<RigidBodyComponent>(entity).kinematic : false;
          const bool isStatic = collider.isStatic && !hasBody;

          auto it = m_bodies.find(key);
          const bool needsRecreate =
              it == m_bodies.end() || it->second.actor == nullptr ||
              it->second.isStatic != isStatic || it->second.kinematic != kinematic ||
              it->second.shapeKind != collider.shape ||
              it->second.halfExtents.x != collider.halfExtents.x ||
              it->second.halfExtents.y != collider.halfExtents.y ||
              it->second.halfExtents.z != collider.halfExtents.z ||
              it->second.scale.x != tf.getScale().x || it->second.scale.y != tf.getScale().y ||
              it->second.scale.z != tf.getScale().z;

          if (needsRecreate)
          {
            if (it != m_bodies.end() && it->second.actor != nullptr)
            {
              it->second.actor->release();
              it->second.actor = nullptr;
            }
            BodyRecord record{};
            record.keyOwned = std::make_unique<EntityKey>(key);
            record.isStatic = isStatic;
            record.kinematic = kinematic;
            record.shapeKind = collider.shape;
            record.halfExtents = collider.halfExtents;
            record.scale = tf.getScale();
            record.commandedPose =
                PxTransform(toPx(tf.getPosition()), yawToQuat(tf.getYaw()));
            createActor(record, collider, hasBody ? &scene.getComponent<RigidBodyComponent>(entity)
                                                  : nullptr);
            m_bodies[key] = std::move(record);
          }
          else
          {
            BodyRecord &record = it->second;
            const PxTransform desired(toPx(tf.getPosition()), yawToQuat(tf.getYaw()));
            const PxVec3 delta = desired.p - record.commandedPose.p;
            const bool poseChanged =
                delta.magnitudeSquared() > 1e-8f ||
                std::fabs(desired.q.x - record.commandedPose.q.x) > 1e-5f ||
                std::fabs(desired.q.y - record.commandedPose.q.y) > 1e-5f ||
                std::fabs(desired.q.z - record.commandedPose.q.z) > 1e-5f ||
                std::fabs(desired.q.w - record.commandedPose.q.w) > 1e-5f;
            if (!poseChanged)
            {
              continue;
            }
            if (record.isStatic || record.kinematic)
            {
              if (record.kinematic)
              {
                static_cast<PxRigidDynamic *>(record.actor)->setKinematicTarget(desired);
              }
              else
              {
                record.actor->setGlobalPose(desired);
              }
              record.commandedPose = desired;
            }
          }
        }

        for (auto it = m_bodies.begin(); it != m_bodies.end();)
        {
          if (seen.find(it->first) == seen.end())
          {
            if (it->second.actor != nullptr)
            {
              it->second.actor->release();
            }
            it = m_bodies.erase(it);
          }
          else
          {
            ++it;
          }
        }
      }

      void createActor(BodyRecord &record, const ColliderComponent &collider,
                       const RigidBodyComponent *body)
      {
        const PxTransform pose = record.commandedPose;
        const float sx = std::fabs(record.scale.x);
        const float sy = std::fabs(record.scale.y);
        const float sz = std::fabs(record.scale.z);

        PxShape *shape = nullptr;
        if (collider.shape == ColliderComponent::Shape::Sphere)
        {
          const float radius = collider.halfExtents.x * std::max({sx, sy, sz});
          if (record.isStatic)
          {
            record.actor = m_physics->createRigidStatic(pose);
          }
          else
          {
            auto *dyn = m_physics->createRigidDynamic(pose);
            if (record.kinematic)
            {
              dyn->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, true);
            }
            record.actor = dyn;
          }
          shape = PxRigidActorExt::createExclusiveShape(
              *record.actor, PxSphereGeometry(std::max(radius, 0.01f)), *m_material);
        }
        else
        {
          const PxVec3 he(std::max(collider.halfExtents.x * sx, 0.01f),
                          std::max(collider.halfExtents.y * sy, 0.01f),
                          std::max(collider.halfExtents.z * sz, 0.01f));
          if (record.isStatic)
          {
            record.actor = m_physics->createRigidStatic(pose);
          }
          else
          {
            auto *dyn = m_physics->createRigidDynamic(pose);
            if (record.kinematic)
            {
              dyn->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, true);
            }
            record.actor = dyn;
          }
          shape = PxRigidActorExt::createExclusiveShape(*record.actor, PxBoxGeometry(he), *m_material);
        }

        if (shape != nullptr)
        {
          if (collider.isTrigger)
          {
            shape->setFlag(PxShapeFlag::eSIMULATION_SHAPE, false);
            shape->setFlag(PxShapeFlag::eTRIGGER_SHAPE, true);
          }
          else
          {
            shape->setFlag(PxShapeFlag::eSIMULATION_SHAPE, true);
            shape->setFlag(PxShapeFlag::eTRIGGER_SHAPE, false);
          }
          shape->setFlag(PxShapeFlag::eSCENE_QUERY_SHAPE, true);
        }
        record.shape = shape;

        if (!record.isStatic && !record.kinematic && body != nullptr)
        {
          auto *dyn = static_cast<PxRigidDynamic *>(record.actor);
          PxRigidBodyExt::updateMassAndInertia(*dyn, std::max(body->mass, 0.01f));
        }

        record.actor->userData = record.keyOwned.get();
        m_scene->addActor(*record.actor);
      }

      PxDefaultAllocator m_allocator{};
      PxDefaultErrorCallback m_errorCallback{};
      PxFoundation *m_foundation = nullptr;
      PxPhysics *m_physics = nullptr;
      PxDefaultCpuDispatcher *m_dispatcher = nullptr;
      PxScene *m_scene = nullptr;
      PxMaterial *m_material = nullptr;
      bool m_valid = false;
      mutable std::unordered_map<EntityKey, BodyRecord, EntityKeyHash> m_bodies;
    };

  } // namespace

  std::unique_ptr<IPhysicsWorld> createPhysXWorld()
  {
    auto world = std::make_unique<PhysXWorld>();
    return world;
  }

} // namespace Nebula
