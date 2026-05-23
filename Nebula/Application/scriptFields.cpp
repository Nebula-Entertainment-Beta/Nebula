#include "scriptFields.h"

#include <iostream>

namespace Nebula
{

  void ScriptFieldRegistry::registerFields(std::string_view scriptId,
                                           const ScriptFieldDescriptor *fields,
                                           std::size_t fieldCount)
  {
    const std::string id(scriptId);
    if (m_fields.find(id) != m_fields.end())
    {
      std::cout << "[ScriptFieldRegistry] Re-registering fields for script: " << id << '\n';
    }
    if (fields == nullptr || fieldCount == 0)
    {
      m_fields[id] = {};
      return;
    }
    m_fields[id] = std::vector<ScriptFieldDescriptor>(fields, fields + fieldCount);
  }

  const std::vector<ScriptFieldDescriptor> *ScriptFieldRegistry::getFields(
      std::string_view scriptId) const
  {
    const auto it = m_fields.find(std::string(scriptId));
    if (it == m_fields.end())
    {
      return nullptr;
    }
    return &it->second;
  }

  bool ScriptFieldRegistry::hasFields(std::string_view scriptId) const
  {
    return m_fields.find(std::string(scriptId)) != m_fields.end();
  }

} // namespace Nebula
