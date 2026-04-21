#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
#include "component.h"



namespace Nebula{

  using EntityID = uint32_t;
  EntityID id;

  struct Entity{
    EntityID id = 0;

  };

  template<typename ComponentType>
  struct ComponentData{
    ComponentType data;

  };

  class Scene{

  public:
   
    Scene() = default;
    Entity createEntity();
    void destroyEntity(Entity entity);
    

    template<typename ComponentType, typename... Args>
    ComponentType& addComponent(Entity entity, Args&&... args);


    template<typename ComponentType>
    ComponentType& getComponent(Entity entity);

    template<typename ComponentType>
    bool hasComponent(Entity entity) const ;

    template<typename ComponentType>
    void removeComponent(Entity entity);

    //validity helpers
    bool isValidEntity(Entity entity) const;
    std::vector<Entity>& getAllEntities() const;



  private:
    std::vector<Entity> m_entities;
    
    
   

  };
}