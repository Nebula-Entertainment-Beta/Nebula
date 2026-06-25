#include "editorApplication.h"
#include "register_Script.h"
#include "sceneDefaults.h"

int main()
{
  Nebula::ApplicationSpec spec;
  spec.title = "Nimbus Editor";

  const std::vector<Editor::ScenePreset> scenePresets = {
      {"Combat Arena", Nimbus::buildCombatArenaScene},
      {"Traversal Test", Nimbus::buildTraversalTestScene},
  };

  Editor::EditorApplication app(
      spec,
      [](auto &registry, auto &fields)
      { Nimbus::registerAllGameplayScripts(registry, fields); },
      Nimbus::buildDefaultScene,
      scenePresets);

  if (!app.getWindow().isValid())
    return 1;
  app.run();
  return 0;
}
