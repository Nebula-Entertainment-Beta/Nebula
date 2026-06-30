#pragma once

#include <functional>
#include <vector>

#include <imgui.h>

#include "application.h"
#include "logSink.h"
#include "editorState.h"
#include "sceneViewPanel.h"
#include "hierarchyPanel.h"
#include "inspectorPanel.h"
#include "consolePanel.h"
#include "editorLog.h"
#include "editorPlayMode.h"
#include "sceneSerializer.h"
#include "editorTemplate.h"
#include "render/sceneViewFrameBuffer.h"

namespace Editor
{
  using DebugPanelDrawer = std::function<void(bool isPlaying)>;
  using ScriptRegistrar = std::function<void(
      Nebula::ScriptRegistry &,
      Nebula::ScriptFieldRegistry &)>;

  /** Game-specific default scene content; supplied by the host executable at startup. */
  using NewSceneBuilder = std::function<void(Nebula::Scene &)>;

  struct ScenePreset
  {
    const char *label = "";
    NewSceneBuilder build;
  };

  /** Forwards engine script logs into the editor Console panel. */
  class EditorLogSink final : public Nebula::ILogSink
  {
  public:
    void setLog(EditorLog *log) { m_log = log; }
    void info(std::string_view msg) override;

  private:
    EditorLog *m_log = nullptr;
  };

  class EditorApplication : public Nebula::Application
  {
  public:
    explicit EditorApplication(const Nebula::ApplicationSpec &spec);
    EditorApplication(const Nebula::ApplicationSpec &spec, ScriptRegistrar registerScripts,
                      NewSceneBuilder buildNewScene = nullptr,
                      std::vector<ScenePreset> scenePresets = {});
    ~EditorApplication() override;
    bool renderSceneToMainFramebuffer() const override { return false; }
    bool loadScene(const std::string_view path);
    bool saveScene();
    void newScene();
    void registerGameSystems() override;
    bool saveSelectedAsPrefab(std::string_view logicalPath);
    Nebula::Entity instantiatePrefab(std::string_view prefabPath);
    void createEntityFromTemplate(const char *templateId);

  protected:
    void onStartup() override;
    void onRender() override;

  private:
    EditorState m_state;
    ScriptRegistrar m_registerScripts;
    NewSceneBuilder m_buildNewScene;
    std::vector<ScenePreset> m_scenePresets;
    bool m_dockLayoutBuilt = false;
    SceneViewPanel m_sceneViewPanel;
    SceneViewFrameBuffer m_sceneViewFrameBuffer;
    HierarchyPanel m_hierarchy;
    InspectorPanel m_inspector;
    ConsolePanel m_console;
    EditorLog m_editorLog;
    EditorLogSink m_scriptLogSink;
    EditorPlayMode m_playmode;
    EditorTemplate m_template;
    Nebula::SceneSerializer m_sceneSerializer;

    void drawDockspace();
    void drawEditorPanels();
    void drawPlayStopToolbar();
    void enterPlayMode();
    void exitPlayMode();
    void openSceneDialog();
    void createEmptyEntity();
    void deleteSelectedEntity();
    void saveSelectedEntityAsPrefab();
    void setupDefaultDockLayout(ImGuiID dockspaceId);
    void newScene(NewSceneBuilder builder);
  };
}
