/**
 * @file script_Registry.cpp
 * @brief Per-instance factory map and `createScript` dispatch.
 *
 * **Note:** Duplicate ids log and replace the factory so hot reload or double registration is visible.
 */
#include "script_Registry.h"
#include <iostream>
#include <string>
#include <unordered_map>

namespace Nebula
{
  void ScriptRegistry::registerScript(std::string_view id, scriptFactoryFunc factory)
  {
    if (id.empty() || !factory)
      return;
    std::string key{id};
    if (auto it = m_factories.find(key); it != m_factories.end())
    {
      std::cout << "[ScriptRegistry] Re-registering script id: " << id << '\n';
      it->second = std::move(factory);
      return;
    }
    m_factories.emplace(std::move(key), std::move(factory));
  }

  ScriptPtr ScriptRegistry::createScript(std::string_view id) const
  {
    auto it = m_factories.find(std::string(id));
    if (it == m_factories.end())
    {
      return {};
    }
    return it->second();
  }

  bool ScriptRegistry::isRegistered(std::string_view id) const
  {
    return m_factories.find(std::string(id)) != m_factories.end();
  }

  std::vector<std::string> ScriptRegistry::registeredScriptIds() const
  {
    std::vector<std::string> ids;
    for (const auto &pair : m_factories)
    {
      ids.push_back(pair.first);
    }
    return ids;
  }

}