/**
 * @file script_Registry.cpp
 * @brief Global registry storage (function-local static map) and `createScript` dispatch.
 *
 * **Note:** Duplicate ids log and replace the factory so hot reload or double registration is visible.
 */
#include "script_Registry.h"
#include <iostream>
#include <string>
#include <unordered_map>

namespace Nebula
{
  namespace
  {
    std::unordered_map<std::string, scriptFactoryFunc> &RegisterStorage()
    {
      static std::unordered_map<std::string, scriptFactoryFunc> s_registry;
      return s_registry;
    }

  }
  void ScriptRegistry::registerScript(std::string_view id, scriptFactoryFunc factory)
  {
    if (id.empty() || !factory)
    {
      return;
    }

    auto &registry = RegisterStorage();
    auto it = registry.find(std::string(id));

    // Duplicate policy: log + replace
    if (it != registry.end())
    {
      std::cout << "[ScriptRegistry] Re-registering script id: " << id << '\n';
      it->second = std::move(factory);
      return;
    }
    registry.emplace(std::string(id), std::move(factory));
  }

  ScriptPtr ScriptRegistry::createScript(std::string_view id) const
  {
    auto &registry = RegisterStorage();
    auto it = registry.find(std::string(id));
    if (it == registry.end())
    {
      std::cerr << "[ScriptRegistry] Unknown script id: " << id << '\n';
      return {};
    }
    return (it->second)();
  }

  bool ScriptRegistry::isRegistered(std::string_view id) const
  {
    const auto &registry = RegisterStorage();
    return registry.find(std::string(id)) != registry.end();
  }

}