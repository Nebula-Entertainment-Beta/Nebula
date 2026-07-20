/**
 * @file registry.h
 * @brief ECS registry: entity lifetime + sparse-set component storages.
 */
#pragma once
#include "component_registry.h"
#include "component.h"
#include "tag_component.h"
#include "physics/physics_component.h"
#include "ecs/component_storage.h"
#include "ecs/entity.h"
#include "prefabInstance.h"
#include <algorithm>
#include <cstdint>
#include <stdexcept>
#include <tuple>
#include <utility>
#include <vector>

namespace Nebula
{

  namespace detail
  {

    using ComponentStorageTuple = std::tuple<
        ComponentStorage<TransformComponent>,
        ComponentStorage<MeshRendererComponent>,
        ComponentStorage<CameraComponent>,
        ComponentStorage<ScriptComponent>,
        ComponentStorage<TagComponent>,
        ComponentStorage<RigidBodyComponent>,
        ComponentStorage<ColliderComponent>,
        ComponentStorage<followTargetComponent>,
        ComponentStorage<PrefabInstanceComponent>,
        ComponentStorage<EnvironmentComponent>>;

    template <typename T>
    struct ComponentTypeIndex;

#define NEBULA_COMPONENT_INDEX(Type, Index)     \
  template <>                                   \
  struct ComponentTypeIndex<Type>               \
  {                                             \
    static constexpr std::size_t value = Index; \
  };

    NEBULA_COMPONENTS_WITH_INDEX(NEBULA_COMPONENT_INDEX)

#undef NEBULA_COMPONENT_INDEX

    template <typename T>
    constexpr std::size_t kStorageIndex = ComponentTypeIndex<T>::value;

    template <typename T>
    struct ViewIteratorEnd
    {
    };

    template <typename T, typename U>
    class TwoComponentView
    {
    public:
      TwoComponentView(const ComponentStorage<T> &a, const ComponentStorage<U> &b) : m_a(a), m_b(b) {}

      class Iterator
      {
      public:
        Iterator(const ComponentStorage<T> &a, const ComponentStorage<U> &b, std::size_t index)
            : m_a(a), m_b(b), m_index(index)
        {
          advanceToValid();
        }

        bool operator!=(const ViewIteratorEnd<T> &) const { return m_index < m_a.entities().size(); }

        struct Entry
        {
          Entity entity;
          const T &first;
          const U &second;
        };

        Entry operator*() const
        {
          const Entity entity = m_a.entities()[m_index];
          return Entry{entity, m_a.dense()[m_index], m_b.get(entity)};
        }

        Iterator &operator++()
        {
          ++m_index;
          advanceToValid();
          return *this;
        }

      private:
        void advanceToValid()
        {
          while (m_index < m_a.entities().size())
          {
            const Entity entity = m_a.entities()[m_index];
            if (m_b.contains(entity))
            {
              return;
            }
            ++m_index;
          }
        }

        const ComponentStorage<T> &m_a;
        const ComponentStorage<U> &m_b;
        std::size_t m_index = 0;
      };

      Iterator begin() const { return Iterator{m_a, m_b, 0}; }
      ViewIteratorEnd<T> end() const { return {}; }

    private:
      const ComponentStorage<T> &m_a;
      const ComponentStorage<U> &m_b;
    };

  } // namespace detail

  class Registry
  {
  public:
    Entity createEntity();
    Entity createEntityWithId(EntityID id, uint32_t generation = 0);
    void destroyEntity(Entity entity);
    bool isValid(Entity entity) const;
    const std::vector<Entity> &entities() const { return m_entities; }
    void clear();
    void setNextEntityId(EntityID nextId) { m_nextEntityId = nextId; }

    template <typename T, typename... Args>
    T &emplace(Entity entity, Args &&...args)
    {
      if (!isValid(entity))
      {
        throw std::runtime_error("Cannot emplace component on invalid entity");
      }
      return storage<T>().emplace(entity, std::forward<Args>(args)...);
    }

    template <typename T>
    T &get(Entity entity)
    {
      if (!isValid(entity))
      {
        throw std::runtime_error("Cannot get component from invalid entity");
      }
      return storage<T>().get(entity);
    }

    template <typename T>
    const T &get(Entity entity) const
    {
      if (!isValid(entity))
      {
        throw std::runtime_error("Cannot get component from invalid entity");
      }
      return storage<T>().get(entity);
    }

    template <typename T>
    bool has(Entity entity) const
    {
      if (!isValid(entity))
      {
        return false;
      }
      return storage<T>().contains(entity);
    }

    template <typename T>
    void removeComponent(Entity entity)
    {
      storage<T>().remove(entity);
    }

    template <typename T>
    ComponentStorage<T> &storage()
    {
      return std::get<detail::kStorageIndex<T>>(m_storages);
    }

    template <typename T>
    const ComponentStorage<T> &storage() const
    {
      return std::get<detail::kStorageIndex<T>>(m_storages);
    }

    template <typename T, typename U>
    detail::TwoComponentView<T, U> view() const
    {
      return detail::TwoComponentView<T, U>{storage<T>(), storage<U>()};
    }

    template <typename Fn>
    void forEachStorage(Fn &&fn)
    {
#define NEBULA_INVOKE_STORAGE(Type, Index) fn(storage<Type>());
      NEBULA_COMPONENTS_WITH_INDEX(NEBULA_INVOKE_STORAGE)
#undef NEBULA_INVOKE_STORAGE
    }

    template <typename Fn>
    void forEachStorage(Fn &&fn) const
    {
#define NEBULA_INVOKE_STORAGE_CONST(Type, Index) fn(storage<Type>());
      NEBULA_COMPONENTS_WITH_INDEX(NEBULA_INVOKE_STORAGE_CONST)
#undef NEBULA_INVOKE_STORAGE_CONST
    }

  private:
    void removeAllComponents(Entity entity);

    std::vector<Entity> m_entities;
    std::vector<uint32_t> m_generations;
    std::vector<EntityID> m_freeIds;
    EntityID m_nextEntityId = 1;
    detail::ComponentStorageTuple m_storages;
  };

} // namespace Nebula
