#include "prefabOverrides.h"

namespace Nebula::PrefabOverrides
{
  namespace
  {
    void mergeParamsJsonField(nlohmann::json &baseComponent, const nlohmann::json &overrideComponent)
    {
      if (!overrideComponent.contains("paramsJson"))
      {
        return;
      }
      if (!baseComponent.contains("paramsJson") || !baseComponent["paramsJson"].is_object())
      {
        baseComponent["paramsJson"] = nlohmann::json::object();
      }
      mergeJsonDeep(baseComponent["paramsJson"], overrideComponent["paramsJson"]);
    }

    nlohmann::json diffParamsJson(const nlohmann::json &baseParams, const nlohmann::json &currentParams)
    {
      nlohmann::json diff = nlohmann::json::object();
      if (!currentParams.is_object())
      {
        return diff;
      }
      const nlohmann::json &baseObj = baseParams.is_object() ? baseParams : nlohmann::json::object();
      for (const auto &[key, value] : currentParams.items())
      {
        if (!baseObj.contains(key) || baseObj[key] != value)
        {
          diff[key] = value;
        }
      }
      return diff;
    }

    nlohmann::json diffComponent(const nlohmann::json &baseComponent, const nlohmann::json &currentComponent,
                                 const char *componentName)
    {
      nlohmann::json diff = nlohmann::json::object();
      if (!currentComponent.is_object())
      {
        return diff;
      }

      if (std::string(componentName) == "ScriptComponent")
      {
        const nlohmann::json baseParams =
            baseComponent.is_object() && baseComponent.contains("paramsJson") ? baseComponent["paramsJson"]
                                                                              : nlohmann::json::object();
        const nlohmann::json currentParams =
            currentComponent.contains("paramsJson") ? currentComponent["paramsJson"] : nlohmann::json::object();
        const nlohmann::json paramsDiff = diffParamsJson(baseParams, currentParams);
        if (!paramsDiff.empty())
        {
          diff["paramsJson"] = paramsDiff;
        }
        return diff;
      }

      if (!baseComponent.is_object())
      {
        return currentComponent;
      }

      for (const auto &[key, value] : currentComponent.items())
      {
        if (!baseComponent.contains(key) || baseComponent[key] != value)
        {
          diff[key] = value;
        }
      }
      return diff;
    }
  } // namespace

  void mergeJsonDeep(nlohmann::json &base, const nlohmann::json &overrides)
  {
    if (!overrides.is_object())
    {
      return;
    }
    if (!base.is_object())
    {
      base = nlohmann::json::object();
    }
    for (const auto &[key, value] : overrides.items())
    {
      if (value.is_object() && base.contains(key) && base[key].is_object())
      {
        mergeJsonDeep(base[key], value);
      }
      else
      {
        base[key] = value;
      }
    }
  }

  nlohmann::json mergeEntityJson(const nlohmann::json &baseEntity, const nlohmann::json &overrides)
  {
    nlohmann::json merged = baseEntity;
    if (!overrides.is_object())
    {
      return merged;
    }
    for (const auto &[componentKey, componentOverrides] : overrides.items())
    {
      if (!merged.contains(componentKey) || !merged[componentKey].is_object())
      {
        merged[componentKey] = componentOverrides;
        continue;
      }
      if (componentKey == "ScriptComponent")
      {
        mergeParamsJsonField(merged[componentKey], componentOverrides);
        if (componentOverrides.contains("scriptName"))
        {
          merged[componentKey]["scriptName"] = componentOverrides["scriptName"];
        }
      }
      else
      {
        mergeJsonDeep(merged[componentKey], componentOverrides);
      }
    }
    return merged;
  }

  nlohmann::json computeEntityDiff(const nlohmann::json &baseEntity, const nlohmann::json &currentEntity)
  {
    nlohmann::json overrides = nlohmann::json::object();
    if (!currentEntity.is_object())
    {
      return overrides;
    }

    for (const auto &[componentKey, currentComponent] : currentEntity.items())
    {
      if (componentKey == "PrefabInstanceComponent" || componentKey == "id")
      {
        continue;
      }
      const nlohmann::json &baseComponent =
          baseEntity.is_object() && baseEntity.contains(componentKey) ? baseEntity[componentKey]
                                                                      : nlohmann::json::object();
      const nlohmann::json componentDiff =
          diffComponent(baseComponent, currentComponent, componentKey.c_str());
      if (!componentDiff.empty())
      {
        overrides[componentKey] = componentDiff;
      }
    }
    return overrides;
  }

  bool overridesContainPath(const nlohmann::json &overrides, const char *componentKey, const char *fieldKey)
  {
    if (!overrides.is_object() || !overrides.contains(componentKey))
    {
      return false;
    }
    const nlohmann::json &component = overrides[componentKey];
    if (!component.is_object())
    {
      return false;
    }
    if (std::string(componentKey) == "ScriptComponent" && std::string(fieldKey).find("paramsJson.") == 0)
    {
      const std::string paramName = std::string(fieldKey).substr(std::string("paramsJson.").size());
      return component.contains("paramsJson") && component["paramsJson"].is_object() &&
             component["paramsJson"].contains(paramName);
    }
    return component.contains(fieldKey);
  }

} // namespace Nebula::PrefabOverrides
