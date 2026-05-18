#include "ecs/registry.h"

namespace Nebula
{

  Entity Registry::createEntity()
  {
    Entity entity{};
    if (!m_freeIds.empty())
    {
      entity.id = m_freeIds.back();
      m_freeIds.pop_back();
    }
    else
    {
      entity.id = m_nextEntityId++;
    }

    if (entity.id >= m_generations.size())
    {
      m_generations.resize(static_cast<std::size_t>(entity.id) + 1, 0);
    }
    entity.generation = m_generations[entity.id];
    m_entities.push_back(entity);
    return entity;
  }

  Entity Registry::createEntityWithId(EntityID id, uint32_t generation)
  {
    if (id >= m_generations.size())
    {
      m_generations.resize(static_cast<std::size_t>(id) + 1, 0);
    }
    m_generations[id] = generation;

    Entity entity{id, generation};
    m_entities.push_back(entity);
    m_nextEntityId = std::max(m_nextEntityId, static_cast<EntityID>(id + 1));
    return entity;
  }

  void Registry::destroyEntity(Entity entity)
  {
    const auto it = std::find_if(m_entities.begin(), m_entities.end(),
                                 [&](const Entity &e) { return e == entity; });
    if (it == m_entities.end())
    {
      return;
    }

    removeAllComponents(entity);

    if (entity.id < m_generations.size())
    {
      ++m_generations[entity.id];
    }
    m_freeIds.push_back(entity.id);
    m_entities.erase(it);
  }

  bool Registry::isValid(Entity entity) const
  {
    if (entity.id >= m_generations.size())
    {
      return false;
    }
    if (entity.generation != m_generations[entity.id])
    {
      return false;
    }
    return std::find_if(m_entities.begin(), m_entities.end(),
                        [&](const Entity &e) { return e == entity; }) != m_entities.end();
  }

  void Registry::clear()
  {
    m_entities.clear();
    m_generations.clear();
    m_freeIds.clear();
    m_nextEntityId = 1;
    forEachStorage([](auto &storage) { storage.clear(); });
  }

  void Registry::removeAllComponents(Entity entity)
  {
    forEachStorage([&](auto &storage) { storage.remove(entity); });
  }

} // namespace Nebula
