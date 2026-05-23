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

  float readScriptParamFloat(std::string_view paramsJson,
                             std::string_view fieldName,
                             float defaultValue);
  int readScriptParamInt(std::string_view paramsJson,
                         std::string_view fieldName,
                         int defaultValue);
  bool readScriptParamBool(std::string_view paramsJson,
                           std::string_view fieldName,
                           bool defaultValue);

  float readScriptParamFloat(std::string_view paramsJson, const ScriptFieldDescriptor &field);
  int readScriptParamInt(std::string_view paramsJson, const ScriptFieldDescriptor &field);
  bool readScriptParamBool(std::string_view paramsJson, const ScriptFieldDescriptor &field);

  /** Ensures paramsJson contains every registered field (missing keys filled with defaults). */
  std::string mergeScriptParamDefaults(std::string_view paramsJson,
                                       const ScriptFieldRegistry &registry,
                                       std::string_view scriptName);

} // namespace Nebula
