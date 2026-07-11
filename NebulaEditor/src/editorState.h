#pragma once

#include <string>
#include <Nebula.h>

namespace Editor
{

  struct EditorState
  {
    Nebula::Entity selectedEntity{};
    std::string scenePath = "scenes/combat_arena.json";
    bool sceneDirty = false;
    bool showColliderGizmos = true;
    bool showTransformGizmo = true;
  };

}