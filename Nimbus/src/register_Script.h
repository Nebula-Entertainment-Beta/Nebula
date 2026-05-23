#pragma once

namespace Nebula
{
  class ScriptRegistry;
  class ScriptFieldRegistry;
}

namespace Nimbus
{
  void registerAllGameplayScripts(Nebula::ScriptRegistry &registry,
                                  Nebula::ScriptFieldRegistry &fieldRegistry);
}
