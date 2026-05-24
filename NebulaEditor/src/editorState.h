#include <string>
#include <Nebula.h>

namespace Editor
{

  struct EditorState
  {
    Nebula::Entity selectedEntity{};
    std::string scenePath = "scenes/editor_scene.json";
    bool sceneDirty = false;
  };

}