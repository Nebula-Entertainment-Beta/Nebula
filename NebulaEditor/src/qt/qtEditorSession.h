#pragma once

#include "application.h"
#include "editorLog.h"
#include "editorPlayMode.h"
#include "editorState.h"
#include "editorTemplate.h"
#include "qt/qtGizmoTool.h"
#include "sceneSerializer.h"

#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace Editor
{

  using ScriptRegistrar = std::function<void(Nebula::ScriptRegistry &, Nebula::ScriptFieldRegistry &)>;
  using NewSceneBuilder = std::function<void(Nebula::Scene &)>;

  struct ScenePreset
  {
    const char *label = "";
    NewSceneBuilder build;
    const char *loadPath = nullptr;
  };

  /**
   * Qt-hosted editor session: reuses Nebula::Application (GLFW hidden) for simulation/render
   * while Qt owns docking UI and transform tools.
   */
  class QtEditorSession
  {
  public:
    QtEditorSession(ScriptRegistrar registerScripts, NewSceneBuilder buildNewScene,
                    std::vector<ScenePreset> presets);
    ~QtEditorSession();

    Nebula::Application &app() { return *m_app; }
    EditorState &state() { return m_state; }
    EditorLog &log() { return m_log; }
    QtGizmoTool &gizmo() { return m_gizmo; }
    const std::vector<ScenePreset> &presets() const { return m_presets; }

    void tick();
    void enterPlay();
    void exitPlay();
    bool isPlaying() const;

    bool loadScene(const std::string &logicalPath);
    bool saveScene();
    bool saveSceneAs(const std::string &absolutePath);
    void newScene();
    void newScene(NewSceneBuilder builder);
    void openSceneDialog();
    void saveSceneAsDialog();

    void createEntity(const char *templateId);
    void deleteSelected();
    Nebula::Entity instantiatePrefab(const std::string &path);

  private:
    class HiddenApp;

    std::unique_ptr<Nebula::Application> m_app;
    ScriptRegistrar m_registerScripts;
    NewSceneBuilder m_buildNewScene;
    std::vector<ScenePreset> m_presets;
    EditorState m_state;
    EditorLog m_log;
    EditorPlayMode m_playMode;
    EditorTemplate m_template;
    Nebula::SceneSerializer m_serializer;
    QtGizmoTool m_gizmo;
    bool m_started = false;
  };

}
