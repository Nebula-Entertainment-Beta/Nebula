/**
 * @file scriptFields.h
 * @brief Per-script field metadata for editor params and paramsJson defaults.
 */
#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace Nebula
{

  enum class ScriptFieldType : std::uint8_t
  {
    Float,
    Int,
    Bool,
  };

  struct ScriptFieldDescriptor
  {
    std::string name;
    ScriptFieldType type = ScriptFieldType::Float;
    bool visibleInEditor = true;
    float defaultFloat = 0.f;
    int defaultInt = 0;
    bool defaultBool = false;
  };

  class ScriptFieldRegistry
  {
  public:
    void registerFields(std::string_view scriptId,
                        const ScriptFieldDescriptor *fields,
                        std::size_t fieldCount);
    const std::vector<ScriptFieldDescriptor> *getFields(std::string_view scriptId) const;
    bool hasFields(std::string_view scriptId) const;

  private:
    std::unordered_map<std::string, std::vector<ScriptFieldDescriptor>> m_fields;
  };

} // namespace Nebula
