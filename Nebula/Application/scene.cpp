#include "scene.h"
#include <algorithm>
#include <fstream>
#include <nlohmann/json.hpp>

namespace Nebula {

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
            [&](const Entity& e) { return e.id == entity.id; }),
            m_entities.end());

        // Also remove all components associated with this entity
        for (auto& [typeIndex, store] : m_componentStores) {
        (void)typeIndex;
        store.erase(entity.id);
        }
    }

    bool Scene::isValidEntity(Entity entity) const
    {
        return std::find_if(m_entities.begin(), m_entities.end(),
            [&](const Entity& e) { return e.id == entity.id; }) != m_entities.end();
    }

    const std::vector<Entity>& Scene::getAllEntities() const
    {
        return m_entities;
    }

    

    void Scene::clear(){
        m_entities.clear();
        m_componentStores.clear();
        m_nextEntityID = 1;
    }

    bool Scene::saveToFile(const std::string& path) const
    {
        nlohmann::json root;
        root["entities"] = nlohmann::json::array();

        for (const Entity& entity : m_entities) {
            nlohmann::json entityJson;
            entityJson["id"] = entity.id;

            const auto transformStoreIt = m_componentStores.find(std::type_index(typeid(TransformComponent)));
            if (transformStoreIt != m_componentStores.end()) {
                const auto entityTransformIt = transformStoreIt->second.find(entity.id);
                if (entityTransformIt != transformStoreIt->second.end()) {
                    const TransformComponent& transformComponent =
                        std::any_cast<const TransformComponent&>(entityTransformIt->second);
                    const glm::vec3 position = transformComponent.transform.getPosition();
                    entityJson["TransformComponent"] = {
                        { "position", { position.x, position.y, position.z } },
                        { "yaw", transformComponent.transform.getYaw() },
                        { "scale", transformComponent.transform.getScale() }
                    };
                }
            }

            const auto meshStoreIt = m_componentStores.find(std::type_index(typeid(MeshRendererComponent)));
            if (meshStoreIt != m_componentStores.end()) {
                const auto entityMeshIt = meshStoreIt->second.find(entity.id);
                if (entityMeshIt != meshStoreIt->second.end()) {
                    const MeshRendererComponent& meshRendererComponent =
                        std::any_cast<const MeshRendererComponent&>(entityMeshIt->second);
                    entityJson["MeshRendererComponent"] = {
                        { "meshID", meshRendererComponent.m_meshID },
                        { "materialID", meshRendererComponent.m_materialID }
                    };
                }
            }

            const auto cameraStoreIt = m_componentStores.find(std::type_index(typeid(CameraComponent)));
            if (cameraStoreIt != m_componentStores.end()) {
                const auto entityCameraIt = cameraStoreIt->second.find(entity.id);
                if (entityCameraIt != cameraStoreIt->second.end()) {
                    const CameraComponent& cameraComponent =
                        std::any_cast<const CameraComponent&>(entityCameraIt->second);
                    entityJson["CameraComponent"] = {
                        { "pivotOffset", { cameraComponent.pivotOffset.x, cameraComponent.pivotOffset.y, cameraComponent.pivotOffset.z } },
                        { "distance", cameraComponent.distance },
                        { "yaw", cameraComponent.yaw },
                        { "pitch", cameraComponent.pitch },
                        { "fov", cameraComponent.fov },
                        { "nearClip", cameraComponent.nearClip },
                        { "farClip", cameraComponent.farClip },
                        { "isPrimary", cameraComponent.isPrimary }
                    };
                }
            }

            const auto scriptStoreIt = m_componentStores.find(std::type_index(typeid(ScriptComponent)));
            if (scriptStoreIt != m_componentStores.end()) {
                const auto entityScriptIt = scriptStoreIt->second.find(entity.id);
                if (entityScriptIt != scriptStoreIt->second.end()) {
                    const ScriptComponent& scriptComponent =
                        std::any_cast<const ScriptComponent&>(entityScriptIt->second);
                    entityJson["ScriptComponent"] = {
                        { "scriptName", scriptComponent.scriptName }
                    };
                }
            }

            root["entities"].push_back(entityJson);
        }

        std::ofstream outFile(path);
        if (!outFile.is_open()) {
            return false;
        }

        outFile << root.dump(2);
        return true;
    }

    bool Scene::loadFromFile(const std::string& path)
    {
        std::ifstream inFile(path);
        if (!inFile.is_open()) {
            return false;
        }

        nlohmann::json root;
        try {
            inFile >> root;
        } catch (...) {
            return false;
        }

        clear();
        EntityID maxID = 0;
        if (!root.contains("entities") || !root["entities"].is_array()) {
            return false;
        }

        for (const auto& entityJson : root["entities"]) {
            if (!entityJson.contains("id")) {
                continue;
            }
            Entity entity;
            entity.id = entityJson["id"].get<EntityID>();
            m_entities.push_back(entity);
            maxID = std::max(maxID, entity.id);

            if (entityJson.contains("TransformComponent")) {
                const auto& transformJson = entityJson["TransformComponent"];
                auto& transformComponent = addComponent<TransformComponent>(entity);

                if (transformJson.contains("position") && transformJson["position"].is_array()
                    && transformJson["position"].size() == 3) {
                    const glm::vec3 position(
                        transformJson["position"][0].get<float>(),
                        transformJson["position"][1].get<float>(),
                        transformJson["position"][2].get<float>());
                    transformComponent.transform.setPosition(position);
                }

                if (transformJson.contains("yaw")) {
                    transformComponent.transform.setYaw(transformJson["yaw"].get<float>());
                }

                if (transformJson.contains("scale")) {
                    transformComponent.transform.setScale(transformJson["scale"].get<float>());
                }
            }

            if (entityJson.contains("MeshRendererComponent")) {
                const auto& meshRendererJson = entityJson["MeshRendererComponent"];
                auto& meshRendererComponent = addComponent<MeshRendererComponent>(entity);

                if (meshRendererJson.contains("meshID")) {
                    meshRendererComponent.m_meshID = meshRendererJson["meshID"].get<uint32_t>();
                }
                if (meshRendererJson.contains("materialID")) {
                    meshRendererComponent.m_materialID = meshRendererJson["materialID"].get<uint32_t>();
                }
            }

            if (entityJson.contains("CameraComponent")) {
                const auto& cameraJson = entityJson["CameraComponent"];
                auto& cameraComponent = addComponent<CameraComponent>(entity);

                if (cameraJson.contains("pivotOffset") && cameraJson["pivotOffset"].is_array()
                    && cameraJson["pivotOffset"].size() == 3) {
                    cameraComponent.pivotOffset = glm::vec3(
                        cameraJson["pivotOffset"][0].get<float>(),
                        cameraJson["pivotOffset"][1].get<float>(),
                        cameraJson["pivotOffset"][2].get<float>());
                }
                if (cameraJson.contains("distance")) {
                    cameraComponent.distance = cameraJson["distance"].get<float>();
                }
                if (cameraJson.contains("yaw")) {
                    cameraComponent.yaw = cameraJson["yaw"].get<float>();
                }
                if (cameraJson.contains("pitch")) {
                    cameraComponent.pitch = cameraJson["pitch"].get<float>();
                }
                if (cameraJson.contains("fov")) {
                    cameraComponent.fov = cameraJson["fov"].get<float>();
                }
                if (cameraJson.contains("nearClip")) {
                    cameraComponent.nearClip = cameraJson["nearClip"].get<float>();
                }
                if (cameraJson.contains("farClip")) {
                    cameraComponent.farClip = cameraJson["farClip"].get<float>();
                }
                if (cameraJson.contains("isPrimary")) {
                    cameraComponent.isPrimary = cameraJson["isPrimary"].get<bool>();
                }
            }

            if (entityJson.contains("ScriptComponent")) {
                const auto& scriptJson = entityJson["ScriptComponent"];
                auto& scriptComponent = addComponent<ScriptComponent>(entity);
                if (scriptJson.contains("scriptName")) {
                    scriptComponent.scriptName = scriptJson["scriptName"].get<std::string>();
                }
            }
        }

        m_nextEntityID = maxID + 1;
        return true;
    }



}