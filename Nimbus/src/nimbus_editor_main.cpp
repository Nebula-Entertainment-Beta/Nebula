#include "qt/qtMainWindow.h"
#include "nimbusRuntime.h"
#include "register_Script.h"
#include "sceneDefaults.h"

#include <QApplication>

namespace
{
  class NimbusEditorApp final : public Editor::EditorApplication
  {
  public:
    NimbusEditorApp(const Nebula::ApplicationSpec &spec,
                    Editor::ScriptRegistrar registerScripts,
                    Editor::NewSceneBuilder buildNewScene,
                    std::vector<Editor::ScenePreset> scenePresets)
        : Editor::EditorApplication(spec, std::move(registerScripts), std::move(buildNewScene),
                                    std::move(scenePresets))
    {
      setGameUserData(&m_runtime);
    }

  private:
    Nimbus::NimbusRuntime m_runtime;
  };
}

int main(int argc, char *argv[])
{
  QApplication qtApp(argc, argv);

  Nebula::ApplicationSpec spec;
  spec.title = "Nimbus Editor";

  const std::vector<Editor::ScenePreset> scenePresets = {
      {"Vertical Slice", Nimbus::buildVerticalSliceScene},
      {"Combat Arena", Nimbus::buildCombatArenaScene},
      {"Traversal Test", Nimbus::buildTraversalTestScene},
      {"Prefab Encounter Test", nullptr, "scenes/encounter_prefab_test.json"},
  };

  auto editor = std::make_unique<NimbusEditorApp>(
      spec,
      [](auto &registry, auto &fields)
      { Nimbus::registerAllGameplayScripts(registry, fields); },
      Nimbus::buildVerticalSliceScene,
      scenePresets);

  Editor::QtMainWindow window(std::move(editor));
  window.show();
  return qtApp.exec();
}
