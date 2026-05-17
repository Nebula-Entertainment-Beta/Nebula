#include "sceneSerializer.h"
#include <nlohmann/json.hpp>
#include <vector>
#include <string>

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

  }

  bool SceneSerializer::save(const Scene &scene, const IAssetProvider &assets, std::string_view logicalPath)
  {

    nlohmann::json root;
    root["version"] = kCurrentVersion;
    root["entities"] = nlohmann::json::array();

    for (const Entity &entity : scene.m_entities)
    {
      nlohmann::json entityJson;
      entityJson["id"] = entity.id;

      const auto transformStoreIt = scene.m_componentStores.find(std::type_index(typeid(TransformComponent)));
      if (transformStoreIt != scene.m_componentStores.end())
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

      const auto meshStoreIt = scene.m_componentStores.find(std::type_index(typeid(MeshRendererComponent)));
      if (meshStoreIt != scene.m_componentStores.end())
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

      const auto cameraStoreIt = scene.m_componentStores.find(std::type_index(typeid(CameraComponent)));
      if (cameraStoreIt != scene.m_componentStores.end())
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

      const auto scriptStoreIt = scene.m_componentStores.find(std::type_index(typeid(ScriptComponent)));
      if (scriptStoreIt != scene.m_componentStores.end())
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

  bool SceneSerializer::load(Scene &scene, const IAssetProvider &assets, std::string_view logicalPath)
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
    const int version = root.value("version", 0); // missing → treat as legacy 0

    if (version < 0 || version > kCurrentVersion)
    {
      return false;
    }

    scene.clear();
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
      scene.m_entities.push_back(entity);
      maxID = std::max(maxID, entity.id);

      if (entityJson.contains("TransformComponent"))
      {
        const auto &transformJson = entityJson["TransformComponent"];
        auto &transformComponent = scene.addComponent<TransformComponent>(entity);

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
        auto &meshRendererComponent = scene.addComponent<MeshRendererComponent>(entity);

        readUIntField(meshRendererJson, "meshID", meshRendererComponent.m_meshID);
        readUIntField(meshRendererJson, "materialID", meshRendererComponent.m_materialID);
      }

      if (entityJson.contains("CameraComponent"))
      {
        const auto &cameraJson = entityJson["CameraComponent"];
        auto &cameraComponent = scene.addComponent<CameraComponent>(entity);

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
        auto &scriptComponent = scene.addComponent<ScriptComponent>(entity);
        readStringField(scriptJson, "scriptName", scriptComponent.scriptName);
      }
    }

    scene.m_nextEntityID = maxID + 1;
    return true;
  }

}