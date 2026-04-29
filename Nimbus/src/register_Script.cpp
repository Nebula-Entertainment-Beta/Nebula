#include "register_Script.h"
#include "script_Registry.h" // from Nebula
#include "player_Script.h"   // your concrete scripts
#include "ground_Script.h"
#include "mainCamera_Script.h"

namespace Nimbus
{
  void registerAllGameplayScripts(Nebula::ScriptRegistry &registry)
  {
    registry.registerScript("Player", []
                            { return std::make_unique<PlayerScript>(); });
    registry.registerScript("Ground", []
                            { return std::make_unique<groundScript>(); });
    registry.registerScript("MainCamera", []
                            { return std::make_unique<MainCameraScript>(); });
  }
} // namespace Nimbus
