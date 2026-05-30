#include "editorApplication.h"

int main()
{
  Nebula::ApplicationSpec spec;
  spec.title = "Nebula Editor";

  // Game-agnostic entry: no scripts or default scene. Each game ships its own
  // editor host executable that links NebulaEditorLib and supplies callbacks.
  Editor::EditorApplication app(spec);

  if (!app.getWindow().isValid())
    return 1;
  app.run();
  return 0;
}
