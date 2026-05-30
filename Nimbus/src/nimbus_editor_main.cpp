#include "editorApplication.h"
#include "register_Script.h"
#include "sceneDefaults.h"

int main()
{
  Nebula::ApplicationSpec spec;
  spec.title = "Nimbus Editor";

  Editor::EditorApplication app(
      spec,
      [](auto &registry, auto &fields)
      { Nimbus::registerAllGameplayScripts(registry, fields); },
      [](Nebula::Scene &scene)
      { Nimbus::buildDefaultScene(scene); });

  if (!app.getWindow().isValid())
    return 1;
  app.run();
  return 0;
}
