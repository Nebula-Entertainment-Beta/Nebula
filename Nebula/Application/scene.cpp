/**
 * @file scene.cpp
 * @brief Entity create/destroy, component storage, and JSON persistence via nlohmann/json.
 *
 * **Note:** Helpers in an anonymous namespace parse JSON fields; scene format is written for the
 * club’s sample levels, not a full generic serializer.
 */
#include "scene.h"
#include "assetProvider.h"
#include <algorithm>
#include <fstream>
#include <nlohmann/json.hpp>

namespace Nebula
{
    namespace
    {

        bool readFloatField(const nlohmann::json &obj, const char *key, float &outValue)
        {
            if (!obj.contains(key) || !obj[key].is_number())
            {
                return false;
            }
            outValue = obj[key].get<float>();
            return true;
        }

        bool readUIntField(const nlohmann::json &obj, const char *key, uint32_t &outValue)
        {
            if (!obj.contains(key) || !obj[key].is_number_unsigned())
            {
                return false;
            }
            outValue = obj[key].get<uint32_t>();
            return true;
        }

        bool readBoolField(const nlohmann::json &obj, const char *key, bool &outValue)
        {
            if (!obj.contains(key) || !obj[key].is_boolean())
            {
                return false;
            }
            outValue = obj[key].get<bool>();
            return true;
        }

        bool readStringField(const nlohmann::json &obj, const char *key, std::string &outValue)
        {
            if (!obj.contains(key) || !obj[key].is_string())
            {
                return false;
            }
            outValue = obj[key].get<std::string>();
            return true;
        }

        bool readVec3ArrayField(const nlohmann::json &obj, const char *key, Vec3 &outValue)
        {
            if (!obj.contains(key) || !obj[key].is_array() || obj[key].size() != 3)
            {
                return false;
            }
            const auto &values = obj[key];
            if (!values[0].is_number() || !values[1].is_number() || !values[2].is_number())
            {
                return false;
            }
            outValue = Vec3{values[0].get<float>(), values[1].get<float>(), values[2].get<float>()};
            return true;
        }

    } // namespace

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

    bool Scene::saveToFile(const IAssetProvider &assets, std::string_view logicalPath) const
    {
        nlohmann::json root;
        root["entities"] = nlohmann::json::array();

        for (const Entity &entity : m_entities)
        {
            nlohmann::json entityJson;
            entityJson["id"] = entity.id;

            const auto transformStoreIt = m_componentStores.find(std::type_index(typeid(TransformComponent)));
            if (transformStoreIt != m_componentStores.end())
            {
                const auto entityTransformIt = transformStoreIt->second.find(entity.id);
                if (entityTransformIt != transformStoreIt->second.end())
                {
                    const TransformComponent &transformComponent =
                        std::any_cast<const TransformComponent &>(entityTransformIt->second);
                    const Vec3 position = transformComponent.transform.getPosition();
                    entityJson["TransformComponent"] = {
                        {"position", {position.x, position.y, position.z}},
                        {"yaw", transformComponent.transform.getYaw()},
                        {"scale", transformComponent.transform.getScale()}};
                }
            }

            const auto meshStoreIt = m_componentStores.find(std::type_index(typeid(MeshRendererComponent)));
            if (meshStoreIt != m_componentStores.end())
            {
                const auto entityMeshIt = meshStoreIt->second.find(entity.id);
                if (entityMeshIt != meshStoreIt->second.end())
                {
                    const MeshRendererComponent &meshRendererComponent =
                        std::any_cast<const MeshRendererComponent &>(entityMeshIt->second);
                    entityJson["MeshRendererComponent"] = {
                        {"meshID", meshRendererComponent.m_meshID},
                        {"materialID", meshRendererComponent.m_materialID}};
                }
            }

            const auto cameraStoreIt = m_componentStores.find(std::type_index(typeid(CameraComponent)));
            if (cameraStoreIt != m_componentStores.end())
            {
                const auto entityCameraIt = cameraStoreIt->second.find(entity.id);
                if (entityCameraIt != cameraStoreIt->second.end())
                {
                    const CameraComponent &cameraComponent =
                        std::any_cast<const CameraComponent &>(entityCameraIt->second);
                    entityJson["CameraComponent"] = {
                        {"pivotOffset", {cameraComponent.pivotOffset.x, cameraComponent.pivotOffset.y, cameraComponent.pivotOffset.z}},
                        {"distance", cameraComponent.distance},
                        {"yaw", cameraComponent.yaw},
                        {"pitch", cameraComponent.pitch},
                        {"fov", cameraComponent.fov},
                        {"nearClip", cameraComponent.nearClip},
                        {"farClip", cameraComponent.farClip},
                        {"isPrimary", cameraComponent.isPrimary}};
                }
            }

            const auto scriptStoreIt = m_componentStores.find(std::type_index(typeid(ScriptComponent)));
            if (scriptStoreIt != m_componentStores.end())
            {
                const auto entityScriptIt = scriptStoreIt->second.find(entity.id);
                if (entityScriptIt != scriptStoreIt->second.end())
                {
                    const ScriptComponent &scriptComponent =
                        std::any_cast<const ScriptComponent &>(entityScriptIt->second);
                    entityJson["ScriptComponent"] = {
                        {"scriptName", scriptComponent.scriptName}};
                }
            }

            root["entities"].push_back(entityJson);
        }

        const std::string text = root.dump(2);
        std::vector<uint8_t> bytes(text.begin(), text.end());
        return assets.writeFile(logicalPath, bytes);
    }

    bool Scene::loadFromFile(const IAssetProvider &assets, std::string_view logicalPath)
    {
        std::vector<uint8_t> bytes;
        if (!assets.readFile(logicalPath, bytes))
        {
            return false;
        }
        nlohmann::json root;
        try
        {
            const std::string json(bytes.begin(), bytes.end());
            root = nlohmann::json::parse(json);
        }
        catch (...)
        {
            return false;
        }
        clear();
        EntityID maxID = 0;
        if (!root.contains("entities") || !root["entities"].is_array())
        {
            return false;
        }

        for (const auto &entityJson : root["entities"])
        {
            if (!entityJson.contains("id") || !entityJson["id"].is_number_unsigned())
            {
                continue;
            }
            Entity entity;
            entity.id = entityJson["id"].get<EntityID>();
            m_entities.push_back(entity);
            maxID = std::max(maxID, entity.id);

            if (entityJson.contains("TransformComponent"))
            {
                const auto &transformJson = entityJson["TransformComponent"];
                auto &transformComponent = addComponent<TransformComponent>(entity);

                Vec3 position;
                if (readVec3ArrayField(transformJson, "position", position))
                {
                    transformComponent.transform.setPosition(position);
                }

                float yaw = transformComponent.transform.getYaw();
                if (readFloatField(transformJson, "yaw", yaw))
                {
                    transformComponent.transform.setYaw(yaw);
                }

                float scale = transformComponent.transform.getScale();
                if (readFloatField(transformJson, "scale", scale))
                {
                    transformComponent.transform.setScale(scale);
                }
            }

            if (entityJson.contains("MeshRendererComponent"))
            {
                const auto &meshRendererJson = entityJson["MeshRendererComponent"];
                auto &meshRendererComponent = addComponent<MeshRendererComponent>(entity);

                readUIntField(meshRendererJson, "meshID", meshRendererComponent.m_meshID);
                readUIntField(meshRendererJson, "materialID", meshRendererComponent.m_materialID);
            }

            if (entityJson.contains("CameraComponent"))
            {
                const auto &cameraJson = entityJson["CameraComponent"];
                auto &cameraComponent = addComponent<CameraComponent>(entity);

                readVec3ArrayField(cameraJson, "pivotOffset", cameraComponent.pivotOffset);
                readFloatField(cameraJson, "distance", cameraComponent.distance);
                readFloatField(cameraJson, "yaw", cameraComponent.yaw);
                readFloatField(cameraJson, "pitch", cameraComponent.pitch);
                readFloatField(cameraJson, "fov", cameraComponent.fov);
                readFloatField(cameraJson, "nearClip", cameraComponent.nearClip);
                readFloatField(cameraJson, "farClip", cameraComponent.farClip);
                readBoolField(cameraJson, "isPrimary", cameraComponent.isPrimary);
            }

            if (entityJson.contains("ScriptComponent"))
            {
                const auto &scriptJson = entityJson["ScriptComponent"];
                auto &scriptComponent = addComponent<ScriptComponent>(entity);
                readStringField(scriptJson, "scriptName", scriptComponent.scriptName);
            }
        }

        m_nextEntityID = maxID + 1;
        return true;
    }

}