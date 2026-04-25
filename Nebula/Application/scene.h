#pragma once

#include <any>
#include <cstdint>
#include <stdexcept>
#include <typeindex>
#include <unordered_map>
#include <utility>
#include <vector>
#include <string>
#include "component.h"


namespace Nebula {

using EntityID = uint32_t;

struct Entity {
    EntityID id = 0;
};

class Scene {
public:
    Scene() = default;

    Entity createEntity();
    void destroyEntity(Entity entity);

    template <typename ComponentType, typename... Args>
    ComponentType& addComponent(Entity entity, Args&&... args)
    {
        if (!isValidEntity(entity)) {
            throw std::runtime_error("Cannot add component to invalid entity");
        }

        auto& store = m_componentStores[std::type_index(typeid(ComponentType))];
        std::any& slot = store[entity.id];
        slot = ComponentType(std::forward<Args>(args)...);
        return std::any_cast<ComponentType&>(slot);
    }

    template <typename ComponentType>
    ComponentType& getComponent(Entity entity)
    {
        if (!isValidEntity(entity)) {
            throw std::runtime_error("Cannot get component from invalid entity");
        }

        auto typeIt = m_componentStores.find(std::type_index(typeid(ComponentType)));
        if (typeIt == m_componentStores.end()) {
            throw std::runtime_error("Component type not found on scene");
        }

        auto entityIt = typeIt->second.find(entity.id);
        if (entityIt == typeIt->second.end()) {
            throw std::runtime_error("Component not found on entity");
        }

        return std::any_cast<ComponentType&>(entityIt->second);
    }

    template <typename ComponentType>
    bool hasComponent(Entity entity) const
    {
        if (!isValidEntity(entity)) {
            return false;
        }

        auto typeIt = m_componentStores.find(std::type_index(typeid(ComponentType)));
        if (typeIt == m_componentStores.end()) {
            return false;
        }

        return typeIt->second.find(entity.id) != typeIt->second.end();
    }

    template <typename ComponentType>
    void removeComponent(Entity entity)
    {
        auto typeIt = m_componentStores.find(std::type_index(typeid(ComponentType)));
        if (typeIt == m_componentStores.end()) {
            return;
        }
        typeIt->second.erase(entity.id);
    }

    bool isValidEntity(Entity entity) const;
    const std::vector<Entity>& getAllEntities() const;
    bool saveToFile(const std::string& path) const;
    bool loadFromFile(const std::string& path);
    void clear();



private:
    std::vector<Entity> m_entities;
    EntityID m_nextEntityID = 1;
    std::unordered_map<std::type_index, std::unordered_map<EntityID, std::any>> m_componentStores;
};

} 