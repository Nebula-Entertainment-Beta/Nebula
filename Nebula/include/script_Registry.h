/**
 * @file script_Registry.h
 * @brief Maps string script names to factories that produce `IScript` instances.
 *
 * **Why:** Registration happens at game startup (`registerScript`); `Scene` only stores names in JSON.
 */
#pragma once
#include "script.h"
#include <functional>
#include <string_view>

namespace Nebula
{
  using scriptFactoryFunc = std::function<ScriptPtr()>;

  class ScriptRegistry
  {
  public:
    void registerScript(std::string_view id, scriptFactoryFunc factory);
    ScriptPtr createScript(std::string_view id) const;
    bool isRegistered(std::string_view id) const;
  };

}