/**
 * @file scriptParams.h
 * @brief Read and merge script parameter overrides stored in ScriptComponent::paramsJson.
 */
#pragma once

#include "scriptFields.h"

#include <string>
#include <string_view>

namespace Nebula
{
  class ScriptParams
  {
  public:
    float readScriptParamFloat(std::string_view paramsJson,
                               std::string_view fieldName,
                               float defaultValue);
    int readScriptParamInt(std::string_view paramsJson,
                           std::string_view fieldName,
                           int defaultValue);
    bool readScriptParamBool(std::string_view paramsJson,
                             std::string_view fieldName,
                             bool defaultValue);

    std::vector<Nebula::Entity> readScriptParamEntityVector(std::string_view paramsJson,
                                                            std::string_view fieldName,
                                                            std::vector<Nebula::Entity> defaultValue);

    float readScriptParamFloat(std::string_view paramsJson, const ScriptFieldDescriptor &field);
    int readScriptParamInt(std::string_view paramsJson, const ScriptFieldDescriptor &field);
    bool readScriptParamBool(std::string_view paramsJson, const ScriptFieldDescriptor &field);

    // FIND A WAY TO READ ALL TYPES OF VECTOR AS PARAMS NOT JUST ONE
    std::vector<int> readScriptIntVector(std::string_view paramsJson, const ScriptFieldDescriptor &field);

    std::vector<Nebula::Entity> readScriptParamEntityVector(std::string_view paramsJson, const ScriptFieldDescriptor &field);

    /** Ensures paramsJson contains every registered field (missing keys filled with defaults). */
    std::string mergeScriptParamDefaults(std::string_view paramsJson,
                                         const ScriptFieldRegistry &registry,
                                         std::string_view scriptName);

    std::string setScriptParamFloat(std::string_view paramsJson, std::string_view fieldName, float value);
    std::string setScriptParamInt(std::string_view paramsJson, std::string_view fieldName, int value);
    std::string setScriptParamBool(std::string_view paramsJson, std::string_view fieldName, bool value);
    std::string setScriptParamEntityVector(std::string_view paramsJson, std::string_view fieldName, const std::vector<Nebula::Entity> &value);
  };

} // namespace Nebula
