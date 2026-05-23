#include "sceneSerializer.h"
#include "assetManager.h"
#include "builtin_assets.h"
#include "component_registry.h"
#include "tag_component.h"
#include "physics/physics_component.h"
#include <nlohmann/json.hpp>
#include <string>
#include <string_view>
#include <vector>

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

    nlohmann::json parseScriptParamsJson(std::string_view paramsJson)
    {
      if (paramsJson.empty())
      {
        return nlohmann::json::object();
      }
      try
      {
        nlohmann::json parsed = nlohmann::json::parse(paramsJson);
        if (parsed.is_object())
        {
          return parsed;
        }
      }
      catch (const nlohmann::json::exception &)
      {
      }
      return nlohmann::json::object();
    }

    void writeTransformComponent(nlohmann::json &entityJson, const TransformComponent &component)
    {
      const Vec3 position = component.transform.getPosition();
      entityJson["TransformComponent"] = {
          {"position", {position.x, position.y, position.z}},
          {"yaw", component.transform.getYaw()},
          {"scale", component.transform.getScale()}};
    }

    void writeMeshRendererComponent(nlohmann::json &entityJson, const MeshRendererComponent &component,
                                    const AssetManager &assetManager)
    {
      nlohmann::json meshJson;
      if (!component.m_meshPath.empty())
      {
        meshJson["meshPath"] = component.m_meshPath;
      }
      else if (const std::string_view meshPath = assetManager.meshPathForHandle(component.m_meshID);
               !meshPath.empty())
      {
        meshJson["meshPath"] = std::string(meshPath);
      }

      if (!component.m_materialPath.empty())
      {
        meshJson["materialPath"] = component.m_materialPath;
      }
      else if (const std::string_view materialPath = assetManager.materialPathForHandle(component.m_materialID);
               !materialPath.empty())
      {
        meshJson["materialPath"] = std::string(materialPath);
      }

      entityJson["MeshRendererComponent"] = std::move(meshJson);
    }

    void writeCameraComponent(nlohmann::json &entityJson, const CameraComponent &component)
    {
      nlohmann::json cameraJson = {
          {"pivotOffset", {component.pivotOffset.x, component.pivotOffset.y, component.pivotOffset.z}},
          {"distance", component.distance},
          {"yaw", component.yaw},
          {"pitch", component.pitch},
          {"fov", component.fov},
          {"nearClip", component.nearClip},
          {"farClip", component.farClip},
          {"isPrimary", component.isPrimary}};
      if (!component.targetTag.empty())
      {
        cameraJson["targetTag"] = component.targetTag;
      }
      if (component.targetEntity.id != 0)
      {
        cameraJson["targetEntityId"] = component.targetEntity.id;
      }
      entityJson["CameraComponent"] = std::move(cameraJson);
    }

    void writeScriptComponent(nlohmann::json &entityJson, const ScriptComponent &component)
    {
      entityJson["ScriptComponent"] = {
          {"scriptName", component.scriptName},
          {"paramsJson", parseScriptParamsJson(component.paramsJson)}};
    }

    void writeTagComponent(nlohmann::json &entityJson, const TagComponent &component)
    {
      entityJson["TagComponent"] = {{"tag", component.tag}};
    }

    void writeRigidBodyComponent(nlohmann::json &entityJson, const RigidBodyComponent &component)
    {
      entityJson["RigidBodyComponent"] = {
          {"mass", component.mass},
          {"kinematic", component.kinematic}};
    }

    void writeColliderComponent(nlohmann::json &entityJson, const ColliderComponent &component)
    {
      const char *shapeName = component.shape == ColliderComponent::Shape::Sphere ? "Sphere" : "Box";
      entityJson["ColliderComponent"] = {
          {"shape", shapeName},
          {"halfExtents",
           {component.halfExtents.x, component.halfExtents.y, component.halfExtents.z}}};
    }

    void loadTransformComponent(Scene &scene, Entity entity, const nlohmann::json &entityJson)
    {
      if (!entityJson.contains("TransformComponent"))
      {
        return;
      }

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

    void loadMeshRendererComponent(Scene &scene, Entity entity, const nlohmann::json &entityJson)
    {
      if (!entityJson.contains("MeshRendererComponent"))
      {
        return;
      }

      const auto &meshRendererJson = entityJson["MeshRendererComponent"];
      auto &meshRendererComponent = scene.addComponent<MeshRendererComponent>(entity);

      readStringField(meshRendererJson, "meshPath", meshRendererComponent.m_meshPath);
      readStringField(meshRendererJson, "materialPath", meshRendererComponent.m_materialPath);

      if (meshRendererComponent.m_meshPath.empty())
      {
        uint32_t legacyMeshId = UINT32_MAX;
        if (readUIntField(meshRendererJson, "meshID", legacyMeshId))
        {
          if (legacyMeshId == 0)
          {
            meshRendererComponent.m_meshPath = kBuiltinMeshCubePath;
          }
          else if (legacyMeshId == 1)
          {
            meshRendererComponent.m_meshPath = kBuiltinMeshGroundPath;
          }
        }
      }

      if (meshRendererComponent.m_materialPath.empty())
      {
        uint32_t legacyMaterialId = UINT32_MAX;
        if (readUIntField(meshRendererJson, "materialID", legacyMaterialId))
        {
          if (legacyMaterialId == 0)
          {
            meshRendererComponent.m_materialPath = kBuiltinMaterialGroundPath;
          }
          else if (legacyMaterialId == 1)
          {
            meshRendererComponent.m_materialPath = kBuiltinMaterialCubePath;
          }
        }
      }
    }

    void loadCameraComponent(Scene &scene, Entity entity, const nlohmann::json &entityJson)
    {
      if (!entityJson.contains("CameraComponent"))
      {
        return;
      }

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
      readStringField(cameraJson, "targetTag", cameraComponent.targetTag);
      if (cameraJson.contains("targetEntityId") && cameraJson["targetEntityId"].is_number_unsigned())
      {
        const EntityID targetId = cameraJson["targetEntityId"].get<EntityID>();
        for (const Entity candidate : scene.getAllEntities())
        {
          if (candidate.id == targetId)
          {
            cameraComponent.targetEntity = candidate;
            break;
          }
        }
      }
    }

    void loadScriptComponent(Scene &scene, Entity entity, const nlohmann::json &entityJson)
    {
      if (!entityJson.contains("ScriptComponent"))
      {
        return;
      }

      const auto &scriptJson = entityJson["ScriptComponent"];
      auto &scriptComponent = scene.addComponent<ScriptComponent>(entity);
      readStringField(scriptJson, "scriptName", scriptComponent.scriptName);

      if (scriptJson.contains("paramsJson"))
      {
        const auto &paramsJson = scriptJson["paramsJson"];
        if (paramsJson.is_object())
        {
          scriptComponent.paramsJson = paramsJson.dump();
        }
        else if (paramsJson.is_string())
        {
          scriptComponent.paramsJson = paramsJson.get<std::string>();
        }
      }
    }

    void loadTagComponent(Scene &scene, Entity entity, const nlohmann::json &entityJson)
    {
      if (entityJson.contains("TagComponent"))
      {
        const auto &tagJson = entityJson["TagComponent"];
        auto &tagComponent = scene.addComponent<TagComponent>(entity);
        readStringField(tagJson, "tag", tagComponent.tag);
        return;
      }

      if (entityJson.contains("tag") && entityJson["tag"].is_string())
      {
        auto &tagComponent = scene.addComponent<TagComponent>(entity);
        tagComponent.tag = entityJson["tag"].get<std::string>();
      }
    }

    void loadRigidBodyComponent(Scene &scene, Entity entity, const nlohmann::json &entityJson)
    {
      if (!entityJson.contains("RigidBodyComponent"))
      {
        return;
      }

      const auto &bodyJson = entityJson["RigidBodyComponent"];
      auto &bodyComponent = scene.addComponent<RigidBodyComponent>(entity);
      readFloatField(bodyJson, "mass", bodyComponent.mass);
      readBoolField(bodyJson, "kinematic", bodyComponent.kinematic);
    }

    void loadColliderComponent(Scene &scene, Entity entity, const nlohmann::json &entityJson)
    {
      if (!entityJson.contains("ColliderComponent"))
      {
        return;
      }

      const auto &colliderJson = entityJson["ColliderComponent"];
      auto &colliderComponent = scene.addComponent<ColliderComponent>(entity);

      if (colliderJson.contains("shape") && colliderJson["shape"].is_string())
      {
        const std::string shape = colliderJson["shape"].get<std::string>();
        if (shape == "Sphere")
        {
          colliderComponent.shape = ColliderComponent::Shape::Sphere;
        }
        else
        {
          colliderComponent.shape = ColliderComponent::Shape::Box;
        }
      }

      readVec3ArrayField(colliderJson, "halfExtents", colliderComponent.halfExtents);
    }

#define NEBULA_SAVE_COMPONENT(Type)                                              \
  if (scene.hasComponent<Type>(entity))                                          \
  {                                                                              \
    write##Type(entityJson, scene.getComponent<Type>(entity));                   \
  }

#define NEBULA_SAVE_MESH_RENDERER(Type)                                          \
  if (scene.hasComponent<Type>(entity))                                          \
  {                                                                              \
    write##Type(entityJson, scene.getComponent<Type>(entity), assetManager);     \
  }

  } // namespace

  bool SceneSerializer::save(const Scene &scene, const AssetManager &assetManager, const IAssetProvider &assets,
                             std::string_view logicalPath)
  {

    nlohmann::json root;
    root["version"] = kCurrentVersion;
    root["entities"] = nlohmann::json::array();

    for (const Entity &entity : scene.getAllEntities())
    {
      nlohmann::json entityJson;
      entityJson["id"] = entity.id;

      NEBULA_SAVE_COMPONENT(TransformComponent)
      NEBULA_SAVE_MESH_RENDERER(MeshRendererComponent)
      NEBULA_SAVE_COMPONENT(CameraComponent)
      NEBULA_SAVE_COMPONENT(ScriptComponent)
      NEBULA_SAVE_COMPONENT(TagComponent)
      NEBULA_SAVE_COMPONENT(RigidBodyComponent)
      NEBULA_SAVE_COMPONENT(ColliderComponent)

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
    const int version = root.value("version", 0);

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
      const EntityID id = entityJson["id"].get<EntityID>();
      const Entity entity = scene.registry().createEntityWithId(id);
      maxID = std::max(maxID, id);

      loadTransformComponent(scene, entity, entityJson);
      loadMeshRendererComponent(scene, entity, entityJson);
      loadCameraComponent(scene, entity, entityJson);
      loadScriptComponent(scene, entity, entityJson);
      loadTagComponent(scene, entity, entityJson);
      loadRigidBodyComponent(scene, entity, entityJson);
      loadColliderComponent(scene, entity, entityJson);
    }

    scene.registry().setNextEntityId(maxID + 1);
    return true;
  }

#undef NEBULA_SAVE_COMPONENT
#undef NEBULA_SAVE_MESH_RENDERER

} // namespace Nebula
