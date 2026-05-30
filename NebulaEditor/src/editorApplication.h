#pragma once

#include <functional>

#include <imgui.h>

#include "application.h"
#include "editorState.h"
#include "sceneViewPanel.h"
#include "hierarchyPanel.h"
#include "inspectorPanel.h"
#include "consolePanel.h"
#include "editorLog.h"
#include "render/sceneViewFrameBuffer.h"

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
    bool renderSceneToMainFramebuffer() const override { return false; }

  protected:
    void onStartup() override;
    void onRender() override;

  private:
    EditorState m_state;
    ScriptRegistrar m_registerScripts;
    bool m_dockLayoutBuilt = false;
    SceneViewPanel m_sceneViewPanel;
    SceneViewFrameBuffer m_sceneViewFrameBuffer;
    HierarchyPanel m_hierarchy;
    InspectorPanel m_inspector;
    ConsolePanel m_console;
    EditorLog m_editorLog;

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
