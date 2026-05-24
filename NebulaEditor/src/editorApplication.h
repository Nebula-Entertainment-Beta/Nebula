#pragma once

#include <functional>

#include <imgui.h>

#include "application.h"
#include "editorState.h"

namespace Editor
{
  using ScriptRegistrar = std::function<void(
      Nebula::ScriptRegistry &,
      Nebula::ScriptFieldRegistry &)>;

  class EditorApplication : public Nebula::Application
  {
  public:
    explicit EditorApplication(const Nebula::ApplicationSpec &spec);
    EditorApplication(const Nebula::ApplicationSpec &spec, ScriptRegistrar registerScripts);
    ~EditorApplication() override;

  protected:
    void onStartup() override;
    void onRender() override;

  private:
    EditorState m_state;
    ScriptRegistrar m_registerScripts;
    bool m_dockLayoutBuilt = false;

    void drawDockspace();
    void drawEditorPanels();
    void drawPlayStopToolbar();
    void newScene();
    void saveScene();
    void openSceneDialog();
    void createEmptyEntity();
    void setupDefaultDockLayout(ImGuiID dockspaceId);
  };
}
