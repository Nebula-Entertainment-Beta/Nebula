#include "scriptParams.h"

#include <nlohmann/json.hpp>

namespace Nebula
{
  namespace
  {
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

  float readScriptParamFloat(std::string_view paramsJson,
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

  int readScriptParamInt(std::string_view paramsJson,
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

  bool readScriptParamBool(std::string_view paramsJson,
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

  float readScriptParamFloat(std::string_view paramsJson, const ScriptFieldDescriptor &field)
  {
    return readScriptParamFloat(paramsJson, field.name, field.defaultFloat);
  }

  int readScriptParamInt(std::string_view paramsJson, const ScriptFieldDescriptor &field)
  {
    return readScriptParamInt(paramsJson, field.name, field.defaultInt);
  }

  bool readScriptParamBool(std::string_view paramsJson, const ScriptFieldDescriptor &field)
  {
    return readScriptParamBool(paramsJson, field.name, field.defaultBool);
  }

  std::string mergeScriptParamDefaults(std::string_view paramsJson,
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
      }
    }
    return merged.dump();
  }

} // namespace Nebula
