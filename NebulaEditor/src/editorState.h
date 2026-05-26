#pragma once

#include <string>
#include <Nebula.h>

namespace Editor
{

  struct EditorState
  {
    Nebula::Entity selectedEntity{};
    std::string scenePath = "scenes/week2_scene.json";
    bool sceneDirty = false;
  };

}