/**
 * @file scene.h
 * @brief Lightweight entity–component scene; storage is sparse-set ECS (`Registry`).
 *
 * **Owns:** `Scene` delegates entity/component operations to `Registry`.
 * Component types are listed in `component_registry.h` (storage, JSON, editor).
 */
#pragma once
#include "component.h"
#include "ecs/entity.h"
#include "ecs/registry.h"

namespace Nebula
{

  class Scene
  {
    friend class SceneSerializer;

  public:
    Scene() = default;

    Entity createEntity() { return m_registry.createEntity(); }
    void destroyEntity(Entity entity) { m_registry.destroyEntity(entity); }

    template <typename ComponentType, typename... Args>
    ComponentType &addComponent(Entity entity, Args &&...args)
    {
      return m_registry.emplace<ComponentType>(entity, std::forward<Args>(args)...);
    }

    template <typename ComponentType>
    ComponentType &getComponent(Entity entity)
    {
      return m_registry.get<ComponentType>(entity);
    }

    template <typename ComponentType>
    const ComponentType &getComponent(Entity entity) const
    {
      return m_registry.get<ComponentType>(entity);
    }

    template <typename ComponentType>
    bool hasComponent(Entity entity) const
    {
      return m_registry.has<ComponentType>(entity);
    }

    template <typename ComponentType>
    void removeComponent(Entity entity)
    {
      m_registry.removeComponent<ComponentType>(entity);
    }

    bool isValidEntity(Entity entity) const { return m_registry.isValid(entity); }
    const std::vector<Entity> &getAllEntities() const { return m_registry.entities(); }
    void clear() { m_registry.clear(); }

    Registry &registry() { return m_registry; }
    const Registry &registry() const { return m_registry; }

  private:
    Registry m_registry;
  };

} // namespace Nebula
