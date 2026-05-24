
#include "register_Script.h"
#include "editorApplication.h"

int main()
{
  Nebula::ApplicationSpec spec;
  spec.title = "Nebula Editor";

  Editor::EditorApplication app(spec, [](auto &registry, auto &fields)
                                { Nimbus::registerAllGameplayScripts(registry, fields); });

  if (!app.getWindow().isValid())
    return 1;
  app.run();
  return 0;
}