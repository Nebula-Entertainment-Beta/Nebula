#include "physics/iphysics_world.h"

#include "component.h"
#include "physics/collision_math.h"
#include "physics/physics_component.h"
#include "scene.h"

#include <algorithm>
#include <cmath>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace Nebula
{

  namespace
  {

    constexpr float kSpatialCellSize = 8.0f;
    constexpr int kMaxCellSpanPerAxis = 4;
    constexpr float kGroundProbeDistance = 0.25f;

    bool passesOverlapFilter(const ColliderComponent &collider, OverlapFilter filter)
    {
      switch (filter)
      {
      case OverlapFilter::All:
        return true;
      case OverlapFilter::SolidOnly:
        return !collider.isTrigger;
      case OverlapFilter::TriggersOnly:
        return collider.isTrigger;
      }
      return true;
    }

    struct CellKey
    {
      int x = 0;
      int y = 0;
      int z = 0;

      bool operator==(const CellKey &other) const
      {
        return x == other.x && y == other.y && z == other.z;
      }
    };

    struct CellKeyHash
    {
      std::size_t operator()(const CellKey &key) const
      {
        const std::size_t hx = static_cast<std::size_t>(key.x) * 73856093u;
        const std::size_t hy = static_cast<std::size_t>(key.y) * 19349663u;
        const std::size_t hz = static_cast<std::size_t>(key.z) * 83492791u;
        return hx ^ hy ^ hz;
      }
    };

    struct PhysicsProxy
    {
      Entity entity{};
      AABB bounds{};
      ColliderComponent collider{};
      Vec3 cachedPosition{};
      float cachedYaw = 0.0f;
      float cachedScale = 1.0f;
      bool dirty = true;
    };

    int cellIndex(float value)
    {
      return static_cast<int>(std::floor(value / kSpatialCellSize));
    }

    AABB makeSphereQueryBounds(const Vec3 &center, float radius)
    {
      return {
          {center.x - radius, center.y - radius, center.z - radius},
          {center.x + radius, center.y + radius, center.z + radius},
      };
    }

    AABB makeBoxQueryBounds(const Vec3 &center, const Vec3 &halfExtents)
    {
      return {
          {center.x - halfExtents.x, center.y - halfExtents.y, center.z - halfExtents.z},
          {center.x + halfExtents.x, center.y + halfExtents.y, center.z + halfExtents.z},
      };
    }

    float axisMin(const AABB &bounds, int axis)
    {
      return axis == 0 ? bounds.min.x : (axis == 1 ? bounds.min.y : bounds.min.z);
    }

    float axisMax(const AABB &bounds, int axis)
    {
      return axis == 0 ? bounds.max.x : (axis == 1 ? bounds.max.y : bounds.max.z);
    }

    float resolveAxisPenetration(const AABB &moving, const AABB &other, int axis, float moveDelta)
    {
      const float movingMin = axisMin(moving, axis);
      const float movingMax = axisMax(moving, axis);
      const float otherMin = axisMin(other, axis);
      const float otherMax = axisMax(other, axis);

      if (movingMax <= otherMin || movingMin >= otherMax)
      {
        return 0.0f;
      }

      if (moveDelta > 0.0f)
      {
        return otherMin - movingMax;
      }
      if (moveDelta < 0.0f)
      {
        return otherMax - movingMin;
      }
      return 0.0f;
    }

    constexpr float kContactSkin = 0.08f;

    bool isStandingOnTop(const AABB &moving, const AABB &other)
    {
      return moving.min.y >= other.max.y - kContactSkin;
    }

    bool colliderDataChanged(const ColliderComponent &previous, const ColliderComponent &current)
    {
      return previous.halfExtents.x != current.halfExtents.x ||
             previous.halfExtents.y != current.halfExtents.y ||
             previous.halfExtents.z != current.halfExtents.z ||
             previous.isTrigger != current.isTrigger || previous.isStatic != current.isStatic ||
             previous.shape != current.shape;
    }

    bool shouldSkipAxisBlock(const AABB &moving, const AABB &other, int axis, float moveDelta)
    {
      if (axis == 1 && moveDelta > 0.0f)
      {
        // Leaving a floor/ledge: ignore colliders whose top is not above our head.
        if (other.max.y <= moving.max.y + kContactSkin)
        {
          return true;
        }
        return false;
      }

      if ((axis == 0 || axis == 2) && isStandingOnTop(moving, other))
      {
        return true;
      }
      return false;
    }

    class SimplePhysicsWorld final : public IPhysicsWorld
    {
    public:
      void step(Scene &scene, float fixedDt) override
      {
        (void)fixedDt;
        syncProxiesFromScene(scene);
        refreshDirtyBounds(scene);
        rebuildSpatialHash();
      }

      void syncTransformsToScene(Scene &) override {}

      void setBodyPosition(Scene &scene, Entity entity, const Vec3 &position) override
      {
        if (!scene.isValidEntity(entity) || !scene.hasComponent<TransformComponent>(entity))
        {
          return;
        }
        scene.getComponent<TransformComponent>(entity).transform.setPosition(position);
        markProxyDirty(entity);
      }

      Vec3 getBodyPosition(const Scene &scene, Entity entity) const override
      {
        if (!scene.isValidEntity(entity) || !scene.hasComponent<TransformComponent>(entity))
        {
          return {};
        }
        return scene.getComponent<TransformComponent>(entity).transform.getPosition();
      }

      bool raycast(const Scene &scene, const Vec3 &origin, const Vec3 &direction, float maxDistance,
                   RaycastHit &out) const override
      {
        ensureSceneCache(scene);

        const Vec3 dir = normalizeOrZero(direction);
        if (dir.x == 0.0f && dir.y == 0.0f && dir.z == 0.0f)
        {
          return false;
        }

        const Ray ray{origin, dir};
        const AABB queryBounds{
            {origin.x - maxDistance, origin.y - maxDistance, origin.z - maxDistance},
            {origin.x + maxDistance, origin.y + maxDistance, origin.z + maxDistance},
        };

        bool found = false;
        float bestT = maxDistance;
        Entity bestEntity{};
        Vec3 bestNormal{};
        bool bestIsSphere = false;
        Vec3 bestSphereCenter{};

        for (const std::size_t proxyIndex : gatherCandidates(queryBounds))
        {
          const PhysicsProxy &proxy = m_proxies[proxyIndex];
          if (proxy.collider.isTrigger)
          {
            continue;
          }

          float t = 0.0f;
          bool hit = false;
          if (proxy.collider.shape == ColliderComponent::Shape::Sphere)
          {
            const Vec3 center = proxy.cachedPosition;
            const float radius = proxy.collider.halfExtents.x * proxy.cachedScale;
            hit = m_collisionMath.rayVsSphere(ray, center, radius, t);
          }
          else
          {
            hit = m_collisionMath.rayVsAABB(ray, proxy.bounds, t);
          }

          if (!hit || t < 0.0f || t > bestT)
          {
            continue;
          }

          found = true;
          bestT = t;
          bestEntity = proxy.entity;
          bestIsSphere = proxy.collider.shape == ColliderComponent::Shape::Sphere;
          if (bestIsSphere)
          {
            bestSphereCenter = proxy.cachedPosition;
          }
        }

        if (!found)
        {
          return false;
        }

        out.entity = bestEntity;
        out.distance = bestT;
        out.point = {origin.x + dir.x * bestT, origin.y + dir.y * bestT, origin.z + dir.z * bestT};
        if (bestIsSphere)
        {
          out.normal = normalizeOrZero(
              {out.point.x - bestSphereCenter.x, out.point.y - bestSphereCenter.y,
               out.point.z - bestSphereCenter.z});
        }
        else
        {
          const std::size_t proxyIndex = findProxyIndex(bestEntity);
          if (proxyIndex != static_cast<std::size_t>(-1))
          {
            out.normal = m_collisionMath.aabbNormalAtPoint(m_proxies[proxyIndex].bounds, out.point);
          }
        }

        return true;
      }

      bool overlapSphere(const Scene &scene, const Vec3 &center, float radius,
                         std::vector<OverlapHit> &out, OverlapFilter filter) const override
      {
        ensureSceneCache(scene);
        out.clear();

        const AABB queryBounds = makeSphereQueryBounds(center, radius);
        const std::vector<std::size_t> candidates = gatherCandidates(queryBounds);
        for (const std::size_t proxyIndex : candidates)
        {
          const PhysicsProxy &proxy = m_proxies[proxyIndex];
          if (!passesOverlapFilter(proxy.collider, filter))
          {
            continue;
          }

          OverlapHit hit{};
          if (!m_collisionMath.sphereVsAABB(center, radius, proxy.bounds, hit))
          {
            continue;
          }
          hit.entity = proxy.entity;
          out.push_back(hit);
        }

        return !out.empty();
      }

      bool overlapBox(const Scene &scene, const Vec3 &center, const Vec3 &halfExtents,
                      std::vector<OverlapHit> &out, OverlapFilter filter) const override
      {
        ensureSceneCache(scene);
        out.clear();

        const AABB queryBounds = makeBoxQueryBounds(center, halfExtents);
        for (const std::size_t proxyIndex : gatherCandidates(queryBounds))
        {
          const PhysicsProxy &proxy = m_proxies[proxyIndex];
          if (!passesOverlapFilter(proxy.collider, filter))
          {
            continue;
          }

          OverlapHit hit{};
          if (!m_collisionMath.aabbVsAABB(queryBounds, proxy.bounds, hit))
          {
            continue;
          }
          hit.entity = proxy.entity;
          out.push_back(hit);
        }
        return !out.empty();
      }

      void moveKinematic(Scene &scene, Entity entity, const Vec3 &delta, bool &outGrounded) override
      {
        outGrounded = false;
        syncProxiesFromScene(scene);
        refreshDirtyBounds(scene);
        rebuildSpatialHash();

        if (!scene.isValidEntity(entity) || !scene.hasComponent<TransformComponent>(entity) ||
            !scene.hasComponent<ColliderComponent>(entity))
        {
          return;
        }

        Transform3D &transform = scene.getComponent<TransformComponent>(entity).transform;
        Vec3 position = transform.getPosition();

        const float axisDelta[3] = {delta.x, delta.y, delta.z};

        constexpr int kAxisOrder[] = {1, 0, 2};

        for (int i = 0; i < 3; ++i)
        {
          const int axis = kAxisOrder[i];
          if (axisDelta[axis] == 0.0f)
          {
            continue;
          }

          float *axisPos = axis == 0 ? &position.x : (axis == 1 ? &position.y : &position.z);
          *axisPos += axisDelta[axis];
          transform.setPosition(position);

          const AABB movedBounds = m_collisionMath.worldAABBFromEntity(scene, entity);
          float axisCorrection = 0.0f;
          bool hasCorrection = false;

          for (const std::size_t proxyIndex : gatherCandidates(movedBounds))
          {
            const PhysicsProxy &proxy = m_proxies[proxyIndex];
            if (proxy.entity == entity || proxy.collider.isTrigger)
            {
              continue;
            }

            if (shouldSkipAxisBlock(movedBounds, proxy.bounds, axis, axisDelta[axis]))
            {
              continue;
            }

            const float penetration =
                resolveAxisPenetration(movedBounds, proxy.bounds, axis, axisDelta[axis]);
            if (penetration == 0.0f)
            {
              continue;
            }

            if (!hasCorrection)
            {
              axisCorrection = penetration;
              hasCorrection = true;
            }
            else if (axisDelta[axis] > 0.0f)
            {
              axisCorrection = std::min(axisCorrection, penetration);
            }
            else if (axisDelta[axis] < 0.0f)
            {
              axisCorrection = std::max(axisCorrection, penetration);
            }

            if (axis == 1 && axisDelta[1] < 0.0f && penetration > 0.0f)
            {
              outGrounded = true;
            }
          }

          if (axis == 0)
          {
            position.x += axisCorrection;
          }
          else if (axis == 1)
          {
            position.y += axisCorrection;
          }
          else
          {
            position.z += axisCorrection;
          }
          transform.setPosition(position);
        }

        markProxyDirty(entity);
        refreshProxyBounds(scene, entity);
        rebuildSpatialHash();

        if (!outGrounded && delta.y <= 0.0f)
        {
          outGrounded = probeGroundedWithDownwardRay(scene, entity);
        }
      }

    private:
      static Vec3 normalizeOrZero(const Vec3 &v)
      {
        const float lenSq = v.x * v.x + v.y * v.y + v.z * v.z;
        if (lenSq <= 1e-8f)
        {
          return {};
        }
        const float invLen = 1.0f / std::sqrt(lenSq);
        return {v.x * invLen, v.y * invLen, v.z * invLen};
      }

      void syncProxiesFromScene(Scene &scene)
      {
        std::unordered_set<EntityID> seenIds;

        for (const auto &entry : scene.registry().view<ColliderComponent, TransformComponent>())
        {
          seenIds.insert(entry.entity.id);
          const Transform3D &transform = entry.second.transform;

          const std::size_t existingIndex = findProxyIndex(entry.entity);
          if (existingIndex == static_cast<std::size_t>(-1))
          {
            PhysicsProxy proxy{};
            proxy.entity = entry.entity;
            proxy.collider = entry.first;
            proxy.cachedPosition = transform.getPosition();
            proxy.cachedYaw = transform.getYaw();
            proxy.cachedScale = transform.getScale();
            proxy.dirty = true;
            m_entityToProxy[entry.entity.id] = m_proxies.size();
            m_proxies.push_back(proxy);
            continue;
          }

          PhysicsProxy &proxy = m_proxies[existingIndex];
          if (colliderDataChanged(proxy.collider, entry.first))
          {
            proxy.dirty = true;
          }
          proxy.collider = entry.first;
          if (proxy.cachedPosition.x != transform.getPosition().x ||
              proxy.cachedPosition.y != transform.getPosition().y ||
              proxy.cachedPosition.z != transform.getPosition().z || proxy.cachedYaw != transform.getYaw() ||
              proxy.cachedScale != transform.getScale())
          {
            proxy.dirty = true;
          }
          proxy.cachedPosition = transform.getPosition();
          proxy.cachedYaw = transform.getYaw();
          proxy.cachedScale = transform.getScale();
        }

        for (std::size_t i = 0; i < m_proxies.size();)
        {
          if (seenIds.count(m_proxies[i].entity.id) == 0 || !scene.isValidEntity(m_proxies[i].entity))
          {
            removeProxyAt(i);
            continue;
          }
          ++i;
        }
      }

      void refreshDirtyBounds(Scene &scene)
      {
        for (PhysicsProxy &proxy : m_proxies)
        {
          // Always recompute bounds so editor collider edits (Fit to Mesh, inspector
          // tweaks) stay in sync with debug gizmos and collision resolution.
          proxy.bounds = m_collisionMath.worldAABBFromEntity(scene, proxy.entity);
          proxy.dirty = false;
        }
      }

      void refreshProxyBounds(Scene &scene, Entity entity)
      {
        const std::size_t index = findProxyIndex(entity);
        if (index == static_cast<std::size_t>(-1))
        {
          return;
        }
        m_proxies[index].bounds = m_collisionMath.worldAABBFromEntity(scene, entity);
        m_proxies[index].dirty = false;
      }

      void rebuildSpatialHash()
      {
        m_spatialHash.clear();
        for (std::size_t i = 0; i < m_proxies.size(); ++i)
        {
          insertProxyIntoHash(i, m_proxies[i].bounds);
        }
      }

      void insertProxyIntoHash(std::size_t proxyIndex, const AABB &bounds)
      {
        int minX = cellIndex(bounds.min.x);
        int minY = cellIndex(bounds.min.y);
        int minZ = cellIndex(bounds.min.z);
        int maxX = cellIndex(bounds.max.x);
        int maxY = cellIndex(bounds.max.y);
        int maxZ = cellIndex(bounds.max.z);

        const auto clampCellSpan = [](int &minCell, int &maxCell) {
          const int span = maxCell - minCell + 1;
          const int maxSpan = kMaxCellSpanPerAxis * 2 + 1;
          if (span > maxSpan)
          {
            const int center = (minCell + maxCell) / 2;
            minCell = center - kMaxCellSpanPerAxis;
            maxCell = center + kMaxCellSpanPerAxis;
          }
        };
        clampCellSpan(minX, maxX);
        clampCellSpan(minY, maxY);
        clampCellSpan(minZ, maxZ);

        for (int x = minX; x <= maxX; ++x)
        {
          for (int y = minY; y <= maxY; ++y)
          {
            for (int z = minZ; z <= maxZ; ++z)
            {
              m_spatialHash[CellKey{x, y, z}].push_back(proxyIndex);
            }
          }
        }
      }

      std::vector<std::size_t> gatherCandidates(const AABB &queryBounds) const
      {
        std::vector<std::size_t> candidates;
        std::unordered_set<std::size_t> seen;

        const int minX = cellIndex(queryBounds.min.x);
        const int minY = cellIndex(queryBounds.min.y);
        const int minZ = cellIndex(queryBounds.min.z);
        const int maxX = cellIndex(queryBounds.max.x);
        const int maxY = cellIndex(queryBounds.max.y);
        const int maxZ = cellIndex(queryBounds.max.z);

        for (int x = minX; x <= maxX; ++x)
        {
          for (int y = minY; y <= maxY; ++y)
          {
            for (int z = minZ; z <= maxZ; ++z)
            {
              const auto it = m_spatialHash.find(CellKey{x, y, z});
              if (it == m_spatialHash.end())
              {
                continue;
              }
              for (const std::size_t proxyIndex : it->second)
              {
                if (seen.insert(proxyIndex).second)
                {
                  candidates.push_back(proxyIndex);
                }
              }
            }
          }
        }

        // Spatial hash can miss large static colliders (e.g. scaled ground planes).
        // Always include every proxy whose cached bounds overlap the query volume.
        for (std::size_t i = 0; i < m_proxies.size(); ++i)
        {
          OverlapHit hit{};
          if (!m_collisionMath.aabbVsAABB(queryBounds, m_proxies[i].bounds, hit))
          {
            continue;
          }
          if (seen.insert(i).second)
          {
            candidates.push_back(i);
          }
        }

        return candidates;
      }

      bool probeGroundedWithDownwardRay(const Scene &scene, Entity entity) const
      {
        Scene &mutableScene = const_cast<Scene &>(scene);
        const AABB bounds = m_collisionMath.worldAABBFromEntity(mutableScene, entity);
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
        if (!raycast(scene, origin, down, kGroundProbeDistance, hit))
        {
          return false;
        }
        return hit.entity != entity && hit.normal.y > 0.5f;
      }

      void ensureSceneCache(const Scene &scene) const
      {
        if (m_proxies.empty())
        {
          auto *self = const_cast<SimplePhysicsWorld *>(this);
          self->syncProxiesFromScene(const_cast<Scene &>(scene));
          self->refreshDirtyBounds(const_cast<Scene &>(scene));
          self->rebuildSpatialHash();
        }
      }

      std::size_t findProxyIndex(Entity entity) const
      {
        const auto it = m_entityToProxy.find(entity.id);
        if (it == m_entityToProxy.end() || it->second >= m_proxies.size())
        {
          return static_cast<std::size_t>(-1);
        }
        if (m_proxies[it->second].entity.generation != entity.generation)
        {
          return static_cast<std::size_t>(-1);
        }
        return it->second;
      }

      void markProxyDirty(Entity entity)
      {
        const std::size_t index = findProxyIndex(entity);
        if (index != static_cast<std::size_t>(-1))
        {
          m_proxies[index].dirty = true;
        }
      }

      void removeProxyAt(std::size_t index)
      {
        const EntityID removedId = m_proxies[index].entity.id;
        if (index + 1 < m_proxies.size())
        {
          m_entityToProxy[m_proxies.back().entity.id] = index;
        }
        m_proxies[index] = std::move(m_proxies.back());
        m_proxies.pop_back();
        m_entityToProxy.erase(removedId);
      }

      mutable CollisionMath m_collisionMath;
      std::vector<PhysicsProxy> m_proxies;
      std::unordered_map<EntityID, std::size_t> m_entityToProxy;
      std::unordered_map<CellKey, std::vector<std::size_t>, CellKeyHash> m_spatialHash;
    };

  } // namespace

  std::unique_ptr<IPhysicsWorld> createSimplePhysicsWorld()
  {
    return std::make_unique<SimplePhysicsWorld>();
  }

} // namespace Nebula
