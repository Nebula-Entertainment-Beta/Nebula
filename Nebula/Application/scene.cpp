/**
 * @file scene.cpp
 * @brief Entity create/destroy, component storage, and JSON persistence via nlohmann/json.
 *
 * **Note:** Helpers in an anonymous namespace parse JSON fields; scene format is written for the
 * club’s sample levels, not a full generic serializer.
 */
#include "scene.h"
#include <algorithm>

namespace Nebula
{

    Entity Scene::createEntity()
    {
        Entity entity;
        entity.id = m_nextEntityID++;
        m_entities.push_back(entity);
        return entity;
    }

    void Scene::destroyEntity(Entity entity)
    {
        m_entities.erase(std::remove_if(m_entities.begin(), m_entities.end(),
                                        [&](const Entity &e)
                                        { return e.id == entity.id; }),
                         m_entities.end());

        // Also remove all components associated with this entity
        for (auto &[typeIndex, store] : m_componentStores)
        {
            (void)typeIndex;
            store.erase(entity.id);
        }
    }

    bool Scene::isValidEntity(Entity entity) const
    {
        return std::find_if(m_entities.begin(), m_entities.end(),
                            [&](const Entity &e)
                            { return e.id == entity.id; }) != m_entities.end();
    }

    const std::vector<Entity> &Scene::getAllEntities() const
    {
        return m_entities;
    }

    void Scene::clear()
    {
        m_entities.clear();
        m_componentStores.clear();
        m_nextEntityID = 1;
    }

}