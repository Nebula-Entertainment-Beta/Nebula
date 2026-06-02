#include "register_Script.h"

#include <cstddef>
#include "scriptFields.h"
#include "script_Registry.h"
#include "player_Script.h"
#include "ground_Script.h"
#include "mainCamera_Script.h"

namespace Nimbus
{
  void registerAllGameplayScripts(Nebula::ScriptRegistry &registry,
                                  Nebula::ScriptFieldRegistry &fieldRegistry)
  {
    static const Nebula::ScriptFieldDescriptor playerFields[] = {
        {.name = "moveSpeed", .type = Nebula::ScriptFieldType::Float, .defaultFloat = 3.f},
    };
    fieldRegistry.registerFields("Player", playerFields, std::size(playerFields));
    registry.registerScript("Player", []
                            { return std::make_unique<PlayerScript>(); });

    registry.registerScript("Ground", []
                            { return std::make_unique<groundScript>(); });

    static const Nebula::ScriptFieldDescriptor cameraFields[] = {
        {.name = "lookSensitivity", .type = Nebula::ScriptFieldType::Float, .defaultFloat = 0.0035f},
        {.name = "zoomSpeed", .type = Nebula::ScriptFieldType::Float, .defaultFloat = 0.6f},
        {.name = "pitchMin", .type = Nebula::ScriptFieldType::Float, .defaultFloat = -1.2f},
        {.name = "pitchMax", .type = Nebula::ScriptFieldType::Float, .defaultFloat = 0.65f},

    };
    fieldRegistry.registerFields("MainCamera", cameraFields, std::size(cameraFields));
    registry.registerScript("MainCamera", []
                            { return std::make_unique<MainCameraScript>(); });
  }
}
