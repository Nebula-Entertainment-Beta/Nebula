

#include <nlohmann/json.hpp>
#include "scriptParams.h"

namespace Nebula
{
  namespace
  {
    nlohmann::json entityVectorToJson(const std::vector<Nebula::Entity> &entities)
    {
      nlohmann::json arr = nlohmann::json::array();
      for (const Nebula::Entity &entity : entities)
      {
        arr.push_back({{"id", entity.id}, {"generation", entity.generation}});
      }
      return arr;
    }

    std::vector<Nebula::Entity> entityVectorFromJson(const nlohmann::json &json)
    {
      std::vector<Nebula::Entity> entities;
      if (!json.is_array())
      {
        return entities;
      }
      for (const nlohmann::json &item : json)
      {
        if (!item.is_object() || !item.contains("id"))
        {
          continue;
        }
        Nebula::Entity entity{};
        entity.id = item["id"].get<Nebula::EntityID>();
        entity.generation = item.value("generation", 0u);
        entities.push_back(entity);
      }
      return entities;
    }

    nlohmann::json parseParamsObject(std::string_view paramsJson)
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
  } // namespace

  float ScriptParams::readScriptParamFloat(std::string_view paramsJson,
                                           std::string_view fieldName,
                                           float defaultValue)
  {
    const nlohmann::json params = parseParamsObject(paramsJson);
    const std::string key(fieldName);
    if (params.contains(key) && params[key].is_number())
    {
      return params[key].get<float>();
    }
    return defaultValue;
  }

  int ScriptParams::readScriptParamInt(std::string_view paramsJson,
                                       std::string_view fieldName,
                                       int defaultValue)
  {
    const nlohmann::json params = parseParamsObject(paramsJson);
    const std::string key(fieldName);
    if (params.contains(key) && params[key].is_number_integer())
    {
      return params[key].get<int>();
    }
    if (params.contains(key) && params[key].is_number())
    {
      return static_cast<int>(params[key].get<float>());
    }
    return defaultValue;
  }

  bool ScriptParams::readScriptParamBool(std::string_view paramsJson,
                                         std::string_view fieldName,
                                         bool defaultValue)
  {
    const nlohmann::json params = parseParamsObject(paramsJson);
    const std::string key(fieldName);
    if (params.contains(key) && params[key].is_boolean())
    {
      return params[key].get<bool>();
    }
    return defaultValue;
  }

  std::vector<Nebula::Entity> ScriptParams::readScriptParamEntityVector(std::string_view paramsJson, std::string_view fieldName, std::vector<Nebula::Entity> defaultValue)
  {
    const nlohmann::json params = parseParamsObject(paramsJson);
    const std::string key(fieldName);
    if (params.contains(key) && params[key].is_array())
    {
      return entityVectorFromJson(params[key]);
    }
    return defaultValue;
  }

  std::vector<Nebula::Entity> ScriptParams::readScriptParamEntityVector(std::string_view paramsJson, const ScriptFieldDescriptor &field)
  {
    return readScriptParamEntityVector(paramsJson, field.name, field.defaultEntityVector);
  }

  float ScriptParams::readScriptParamFloat(std::string_view paramsJson, const ScriptFieldDescriptor &field)
  {
    return readScriptParamFloat(paramsJson, field.name, field.defaultFloat);
  }

  int ScriptParams::readScriptParamInt(std::string_view paramsJson, const ScriptFieldDescriptor &field)
  {
    return readScriptParamInt(paramsJson, field.name, field.defaultInt);
  }

  bool ScriptParams::readScriptParamBool(std::string_view paramsJson, const ScriptFieldDescriptor &field)
  {
    return readScriptParamBool(paramsJson, field.name, field.defaultBool);
  }

  std::string ScriptParams::mergeScriptParamDefaults(std::string_view paramsJson,
                                                     const ScriptFieldRegistry &registry,
                                                     std::string_view scriptName)
  {
    nlohmann::json merged = parseParamsObject(paramsJson);
    const std::vector<ScriptFieldDescriptor> *fields = registry.getFields(scriptName);
    if (fields == nullptr)
    {
      return merged.dump();
    }

    for (const ScriptFieldDescriptor &field : *fields)
    {
      if (merged.contains(field.name))
      {
        continue;
      }
      switch (field.type)
      {
      case ScriptFieldType::Float:
        merged[field.name] = field.defaultFloat;
        break;
      case ScriptFieldType::Int:
        merged[field.name] = field.defaultInt;
        break;
      case ScriptFieldType::Bool:
        merged[field.name] = field.defaultBool;
        break;
      case ScriptFieldType::EntityVector:
        merged[field.name] = entityVectorToJson(field.defaultEntityVector);
        break;
      }
    }
    return merged.dump();
  }

  std::string ScriptParams::setScriptParamFloat(std::string_view paramsJson, std::string_view fieldName, float value)
  {
    nlohmann::json params = parseParamsObject(paramsJson);
    params[std::string(fieldName)] = value;
    return params.dump();
  }

  std::string ScriptParams::setScriptParamInt(std::string_view paramsJson, std::string_view fieldName, int value)
  {
    nlohmann::json params = parseParamsObject(paramsJson);
    params[std::string(fieldName)] = value;
    return params.dump();
  }

  std::string ScriptParams::setScriptParamBool(std::string_view paramsJson, std::string_view fieldName, bool value)
  {
    nlohmann::json params = parseParamsObject(paramsJson);
    params[std::string(fieldName)] = value;
    return params.dump();
  }

  std::string ScriptParams::setScriptParamEntityVector(std::string_view paramsJson, std::string_view fieldName, const std::vector<Nebula::Entity> &value)
  {
    nlohmann::json params = parseParamsObject(paramsJson);
    params[std::string(fieldName)] = entityVectorToJson(value);
    return params.dump();
  }

} // namespace Nebula
