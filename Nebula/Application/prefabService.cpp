#include "prefabService.h"

#include "component.h"
#include "entityComponentJson_internal.h"
#include "prefabOverrides.h"
#include "prefabSerializer.h"
#include "renderResources.h"
#include "tag_component.h"
#include "physics/physics_component.h"

#include <nlohmann/json.hpp>
#include <string>
#include <unordered_set>
#include <vector>

namespace Nebula
{
  namespace
  {
    nlohmann::json parseOverridesObject(std::string_view overridesJson)
    {
      if (overridesJson.empty())
      {
        return nlohmann::json::object();
      }
      try
      {
        nlohmann::json parsed = nlohmann::json::parse(overridesJson);
        if (parsed.is_object())
        {
          return parsed;
        }
      }
      catch (...)
      {
      }
      return nlohmann::json::object();
    }

    std::optional<nlohmann::json> resolvePrefabEntityJsonRecursive(const IAssetProvider &fileProvider,
                                                                   std::string_view prefabPath,
                                                                   std::unordered_set<std::string> &visited)
    {
      const std::string pathKey(prefabPath);
      if (visited.find(pathKey) != visited.end())
      {
        return std::nullopt;
      }
      visited.insert(pathKey);

      std::string rootText;
      if (!PrefabSerializer::loadRootJsonString(fileProvider, prefabPath, rootText))
      {
        return std::nullopt;
      }

      nlohmann::json root;
      try
      {
        root = nlohmann::json::parse(rootText);
      }
      catch (...)
      {
        return std::nullopt;
      }

      nlohmann::json entityJson = nlohmann::json::object();
      if (root.contains("basePrefab") && root["basePrefab"].is_string())
      {
        const std::string basePath = root["basePrefab"].get<std::string>();
        const auto baseResolved = resolvePrefabEntityJsonRecursive(fileProvider, basePath, visited);
        if (!baseResolved.has_value())
        {
          return std::nullopt;
        }
        entityJson = *baseResolved;
        if (root.contains("overrides") && root["overrides"].is_object())
        {
          entityJson = PrefabOverrides::mergeEntityJson(entityJson, root["overrides"]);
        }
      }
      else if (root.contains("entity") && root["entity"].is_object())
      {
        entityJson = root["entity"];
      }
      else
      {
        return std::nullopt;
      }

      return entityJson;
    }

    void stripPrefabComponents(Scene &scene, Entity entity)
    {
      if (scene.hasComponent<PrefabInstanceComponent>(entity))
      {
        scene.removeComponent<PrefabInstanceComponent>(entity);
      }
      if (scene.hasComponent<TransformComponent>(entity))
      {
        scene.removeComponent<TransformComponent>(entity);
      }
      if (scene.hasComponent<MeshRendererComponent>(entity))
      {
        scene.removeComponent<MeshRendererComponent>(entity);
      }
      if (scene.hasComponent<CameraComponent>(entity))
      {
        scene.removeComponent<CameraComponent>(entity);
      }
      if (scene.hasComponent<ScriptComponent>(entity))
      {
        scene.removeComponent<ScriptComponent>(entity);
      }
      if (scene.hasComponent<TagComponent>(entity))
      {
        scene.removeComponent<TagComponent>(entity);
      }
      if (scene.hasComponent<RigidBodyComponent>(entity))
      {
        scene.removeComponent<RigidBodyComponent>(entity);
      }
      if (scene.hasComponent<ColliderComponent>(entity))
      {
        scene.removeComponent<ColliderComponent>(entity);
      }
      if (scene.hasComponent<EnvironmentComponent>(entity))
      {
        scene.removeComponent<EnvironmentComponent>(entity);
      }
      if (scene.hasComponent<followTargetComponent>(entity))
      {
        scene.removeComponent<followTargetComponent>(entity);
      }
    }

    void applyEntityJsonToInstance(Scene &scene, Entity entity, const nlohmann::json &entityJson,
                                   std::string_view prefabPath, std::string_view overridesJson)
    {
      stripPrefabComponents(scene, entity);
      EntityComponentJson::deserializeEntityFromJson(scene, entity, entityJson);

      auto &inst = scene.addComponent<PrefabInstanceComponent>(entity);
      inst.prefabPath = std::string(prefabPath);
      inst.overridesJson = std::string(overridesJson);
    }

    void setOverrideValue(nlohmann::json &overrides, const char *componentKey, const char *fieldKey,
                          const nlohmann::json &value)
    {
      if (std::string(componentKey) == "ScriptComponent" &&
          std::string(fieldKey).find("paramsJson.") == 0)
      {
        const std::string paramName = std::string(fieldKey).substr(std::string("paramsJson.").size());
        if (!overrides.contains("ScriptComponent"))
        {
          overrides["ScriptComponent"] = nlohmann::json::object();
        }
        if (!overrides["ScriptComponent"].contains("paramsJson"))
        {
          overrides["ScriptComponent"]["paramsJson"] = nlohmann::json::object();
        }
        overrides["ScriptComponent"]["paramsJson"][paramName] = value;
        return;
      }

      if (!overrides.contains(componentKey))
      {
        overrides[componentKey] = nlohmann::json::object();
      }
      overrides[componentKey][fieldKey] = value;
    }
  } // namespace

  std::optional<std::string> PrefabService::resolvePrefabEntityJsonString(const IAssetProvider &fileProvider,
                                                                          std::string_view prefabPath)
  {
    std::unordered_set<std::string> visited;
    const auto resolved = resolvePrefabEntityJsonRecursive(fileProvider, prefabPath, visited);
    if (!resolved.has_value())
    {
      return std::nullopt;
    }
    return resolved->dump();
  }

  Entity PrefabService::instantiate(Scene &scene, AssetManager &assets,
                                    const IAssetProvider &fileProvider,
                                    IRenderResourceFactory &renderResources,
                                    std::string_view prefabPath)
  {
    const auto resolved = resolvePrefabEntityJsonString(fileProvider, prefabPath);
    if (!resolved.has_value())
    {
      return {};
    }

    nlohmann::json entityJson;
    try
    {
      entityJson = nlohmann::json::parse(*resolved);
    }
    catch (...)
    {
      return {};
    }

    const Entity entity = scene.createEntity();
    applyEntityJsonToInstance(scene, entity, entityJson, prefabPath, "{}");
    assets.resolveScene(scene, renderResources);
    return entity;
  }

  bool PrefabService::revertInstance(Scene &scene, Entity entity, AssetManager &assets,
                                     const IAssetProvider &fileProvider,
                                     IRenderResourceFactory &renderResources)
  {
    if (!scene.isValidEntity(entity) || !scene.hasComponent<PrefabInstanceComponent>(entity))
    {
      return false;
    }

    const auto &inst = scene.getComponent<PrefabInstanceComponent>(entity);
    const auto resolved = resolvePrefabEntityJsonString(fileProvider, inst.prefabPath);
    if (!resolved.has_value())
    {
      return false;
    }

    nlohmann::json entityJson;
    try
    {
      entityJson = nlohmann::json::parse(*resolved);
    }
    catch (...)
    {
      return false;
    }

    applyEntityJsonToInstance(scene, entity, entityJson, inst.prefabPath, "{}");
    assets.resolveScene(scene, renderResources);
    return true;
  }

  std::string PrefabService::computeInstanceOverridesJson(const Scene &scene, Entity entity,
                                                          const AssetManager &assets,
                                                          const IAssetProvider &fileProvider)
  {
    if (!scene.isValidEntity(entity) || !scene.hasComponent<PrefabInstanceComponent>(entity))
    {
      return "{}";
    }

    const auto &inst = scene.getComponent<PrefabInstanceComponent>(entity);
    const auto resolved = resolvePrefabEntityJsonString(fileProvider, inst.prefabPath);
    if (!resolved.has_value())
    {
      return "{}";
    }

    nlohmann::json baseEntity;
    try
    {
      baseEntity = nlohmann::json::parse(*resolved);
    }
    catch (...)
    {
      return "{}";
    }

    const nlohmann::json current = EntityComponentJson::serializeEntityToJson(scene, entity, assets, {
                                                                                                            .includeRuntimeIds = false,
                                                                                                            .includePrefabInstance = false,
                                                                                                            .includeCameraEntityRefs = false,
                                                                                                        });
    return PrefabOverrides::computeEntityDiff(baseEntity, current).dump();
  }

  bool PrefabService::saveVariantFromInstance(const Scene &scene, Entity entity, const AssetManager &assets,
                                              const IAssetProvider &fileProvider, std::string_view variantPath)
  {
    if (!scene.isValidEntity(entity) || !scene.hasComponent<PrefabInstanceComponent>(entity))
    {
      return false;
    }

    const auto &inst = scene.getComponent<PrefabInstanceComponent>(entity);
    const std::string overridesJson = computeInstanceOverridesJson(scene, entity, assets, fileProvider);
    return PrefabSerializer::saveVariant(fileProvider, variantPath, inst.prefabPath, overridesJson);
  }

  void PrefabService::setInstanceOverrideFieldJson(PrefabInstanceComponent &instance, const char *componentKey,
                                                 const char *fieldKey, std::string_view jsonValue)
  {
    nlohmann::json overrides = parseOverridesObject(instance.overridesJson);
    nlohmann::json value = nlohmann::json::parse(std::string(jsonValue));
    setOverrideValue(overrides, componentKey, fieldKey, value);
    instance.overridesJson = overrides.dump();
  }

  void PrefabService::setInstanceOverrideField(PrefabInstanceComponent &instance, const char *componentKey,
                                               const char *fieldKey, float value)
  {
    setInstanceOverrideFieldJson(instance, componentKey, fieldKey, std::to_string(value));
  }

  void PrefabService::setInstanceOverrideField(PrefabInstanceComponent &instance, const char *componentKey,
                                               const char *fieldKey, const float *vec3Value)
  {
    nlohmann::json arr = nlohmann::json::array({vec3Value[0], vec3Value[1], vec3Value[2]});
    setInstanceOverrideFieldJson(instance, componentKey, fieldKey, arr.dump());
  }

  void PrefabService::setInstanceOverrideField(PrefabInstanceComponent &instance, const char *componentKey,
                                               const char *fieldKey, int value)
  {
    setInstanceOverrideFieldJson(instance, componentKey, fieldKey, std::to_string(value));
  }

  void PrefabService::setInstanceOverrideField(PrefabInstanceComponent &instance, const char *componentKey,
                                               const char *fieldKey, bool value)
  {
    setInstanceOverrideFieldJson(instance, componentKey, fieldKey, value ? "true" : "false");
  }

  void PrefabService::clearInstanceOverrideField(PrefabInstanceComponent &instance, const char *componentKey,
                                                 const char *fieldKey)
  {
    nlohmann::json overrides = parseOverridesObject(instance.overridesJson);
    if (!overrides.contains(componentKey) || !overrides[componentKey].is_object())
    {
      return;
    }

    if (std::string(componentKey) == "ScriptComponent" &&
        std::string(fieldKey).find("paramsJson.") == 0)
    {
      const std::string paramName = std::string(fieldKey).substr(std::string("paramsJson.").size());
      if (overrides["ScriptComponent"].contains("paramsJson") &&
          overrides["ScriptComponent"]["paramsJson"].is_object())
      {
        overrides["ScriptComponent"]["paramsJson"].erase(paramName);
        if (overrides["ScriptComponent"]["paramsJson"].empty())
        {
          overrides["ScriptComponent"].erase("paramsJson");
        }
      }
    }
    else
    {
      overrides[componentKey].erase(fieldKey);
    }

    if (overrides[componentKey].empty())
    {
      overrides.erase(componentKey);
    }
    instance.overridesJson = overrides.dump();
  }

  bool PrefabService::overridesContainPath(std::string_view overridesJson, const char *componentKey,
                                           const char *fieldKey)
  {
    return PrefabOverrides::overridesContainPath(parseOverridesObject(overridesJson), componentKey, fieldKey);
  }

  bool PrefabService::resetInstanceOverrideField(Scene &scene, Entity entity, const AssetManager &assets,
                                                 const IAssetProvider &fileProvider, const char *componentKey,
                                                 const char *fieldKey)
  {
    (void)assets;
    if (!scene.isValidEntity(entity) || !scene.hasComponent<PrefabInstanceComponent>(entity))
    {
      return false;
    }

    auto &inst = scene.getComponent<PrefabInstanceComponent>(entity);
    const auto resolved = resolvePrefabEntityJsonString(fileProvider, inst.prefabPath);
    if (!resolved.has_value())
    {
      return false;
    }

    nlohmann::json resolvedEntity;
    try
    {
      resolvedEntity = nlohmann::json::parse(*resolved);
    }
    catch (...)
    {
      return false;
    }

    clearInstanceOverrideField(inst, componentKey, fieldKey);

    if (std::string(componentKey) == "TransformComponent" &&
        scene.hasComponent<TransformComponent>(entity) &&
        resolvedEntity.contains("TransformComponent"))
    {
      const auto &transformJson = resolvedEntity["TransformComponent"];
      auto &transform = scene.getComponent<TransformComponent>(entity).transform;
      if (std::string(fieldKey) == "position" && transformJson.contains("position"))
      {
        const auto &pos = transformJson["position"];
        if (pos.is_array() && pos.size() == 3)
        {
          transform.setPosition({pos[0].get<float>(), pos[1].get<float>(), pos[2].get<float>()});
        }
      }
      else if (std::string(fieldKey) == "yaw" && transformJson.contains("yaw"))
      {
        transform.setYaw(transformJson["yaw"].get<float>());
      }
      else if (std::string(fieldKey) == "scale" && transformJson.contains("scale"))
      {
        const auto &scaleJson = transformJson["scale"];
        if (scaleJson.is_number())
        {
          const float uniform = scaleJson.get<float>();
          transform.setScale(uniform);
        }
        else if (scaleJson.is_array() && scaleJson.size() == 3)
        {
          transform.setScale({scaleJson[0].get<float>(), scaleJson[1].get<float>(), scaleJson[2].get<float>()});
        }
      }
      return true;
    }

    if (std::string(componentKey) == "ScriptComponent" && scene.hasComponent<ScriptComponent>(entity) &&
        resolvedEntity.contains("ScriptComponent") &&
        std::string(fieldKey).find("paramsJson.") == 0)
    {
      const std::string paramName = std::string(fieldKey).substr(std::string("paramsJson.").size());
      const auto &scriptJson = resolvedEntity["ScriptComponent"];
      if (!scriptJson.contains("paramsJson") || !scriptJson["paramsJson"].is_object())
      {
        return true;
      }
      const auto &baseParams = scriptJson["paramsJson"];
      auto &script = scene.getComponent<ScriptComponent>(entity);
      nlohmann::json params = nlohmann::json::parse(script.paramsJson.empty() ? "{}" : script.paramsJson);
      if (baseParams.contains(paramName))
      {
        params[paramName] = baseParams[paramName];
      }
      else
      {
        params.erase(paramName);
      }
      script.paramsJson = params.dump();
      return true;
    }

    return false;
  }

} // namespace Nebula
