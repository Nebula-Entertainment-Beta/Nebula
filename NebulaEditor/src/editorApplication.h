#pragma once

#include <functional>
#include <vector>

#include "application.h"
#include "logSink.h"
#include "editorState.h"
#include "editor/assetCatalog.h"
#include "editorLog.h"
#include "editorPlayMode.h"
#include "sceneSerializer.h"
#include "editorTemplate.h"
#include "qt/qtGizmoTool.h"

namespace Editor
{
  using ScriptRegistrar = std::function<void(
      Nebula::ScriptRegistry &,
      Nebula::ScriptFieldRegistry &)>;

  /** Game-specific default scene content; supplied by the host executable at startup. */
  using NewSceneBuilder = std::function<void(Nebula::Scene &)>;

  struct ScenePreset
  {
    const char *label = "";
    NewSceneBuilder build;
    /** When non-null, loads this scene file instead of calling @p build. */
    const char *loadPath = nullptr;
  };

  /** Forwards engine script logs into the editor Console panel. */
  class EditorLogSink final : public Nebula::ILogSink
  {
  public:
    void setLog(EditorLog *log) { m_log = log; }
    void info(std::string_view msg) override;
    void warn(std::string_view msg) override;
    void error(std::string_view msg) override;

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
    bool loadScene(const std::string_view path);
    bool saveScene();
    void newScene();
    void registerGameSystems() override;
    bool saveSelectedAsPrefab(std::string_view logicalPath);
    Nebula::Entity instantiatePrefab(std::string_view prefabPath);
    void revertSelectedPrefabInstance();
    void createVariantFromSelectedInstance();
    void createEntityFromTemplate(const char *templateId);

    EditorState &editorState() { return m_state; }
    EditorLog &editorLog() { return m_editorLog; }
    void setGizmoMode(int mode);
    int gizmoMode() const { return m_qtGizmoMode; }
    QtGizmoTool &qtGizmo() { return m_qtGizmo; }
    Nebula::Scene &scene() { return getScene(); }
    float lastFrameDt() const { return m_lastFrameDt; }
    Nebula::ScriptRegistry &scriptRegistryRef() { return getScriptRegistry(); }
    Nebula::ScriptFieldRegistry &fieldRegistryRef() { return getScriptFieldRegistry(); }
    Nebula::AssetManager &assetManagerRef() { return getAssetManager(); }
    void resolveSceneAssetsNow() { resolveSceneAssets(); }
    bool playing() const { return isPlaying(); }
    const std::vector<ScenePreset> &scenePresets() const { return m_scenePresets; }
    const AssetCatalog &assetCatalog() const { return m_assetCatalog; }
    void refreshAssetsPublic() { refreshAssetCatalog(); }
    void spawnMeshPublic(std::string_view meshPath, std::string_view materialPath,
                         const Nebula::Vec3 &position)
    {
      spawnMeshAt(meshPath, materialPath, position);
    }
    void spawnPrefabPublic(std::string_view prefabPath, const Nebula::Vec3 &position)
    {
      spawnPrefabAt(prefabPath, position);
    }
    void newSceneFromPathPublic(std::string_view path) { newSceneFromPath(path); }
    void saveSelectedAsPrefabPublic() { saveSelectedEntityAsPrefab(); }
    void revertSelectedPrefabPublic() { revertSelectedPrefabInstance(); }
    void createVariantPublic() { createVariantFromSelectedInstance(); }
    bool resetSelectedOverridePublic(const char *component, const char *field);
    Nebula::Input &inputRef() { return getInput(); }
    Nebula::ActionMapping &actionsRef() { return getActionMapping(); }

    // Qt forwards events from the embedded native Scene View directly. QWindow's
    // foreign-window wrapper does not reliably preserve GLFW callbacks on Windows.
    void scenePointerPress(float x, float y, bool left, bool right, bool shift,
                           int width, int height);
    void scenePointerMove(float x, float y, bool left, bool right, int width, int height);
    void scenePointerRelease(float x, float y, bool left, bool right, int width, int height);
    void sceneWheel(float delta);
    void sceneKeyChanged(int key, bool down);
    void sceneDropAsset(std::string_view path, AssetEntryKind kind, float x, float y,
                        int width, int height);

    bool loadScenePublic(const std::string_view path) { return loadScene(path); }
    bool saveScenePublic() { return saveScene(); }
    bool saveSceneAsAbsolutePublic(const std::string &path);
    bool loadSceneAbsolutePublic(const std::string &path);
    void newScenePublic() { newScene(); }
    void newScenePublic(NewSceneBuilder builder) { newScene(std::move(builder)); }
    void enterPlayPublic() { enterPlayMode(); }
    void exitPlayPublic() { exitPlayMode(); }
    void createEntityPublic(const char *id) { createEntityFromTemplate(id); }
    void deleteSelectedPublic() { deleteSelectedEntity(); }

  protected:
    void onStartup() override;
    void onRender() override;
    /** Play uses the game camera; edit mode draws with the editor fly camera in onRender. */
    bool renderSceneToMainFramebuffer() const override { return isPlaying(); }

  private:
    EditorState m_state;
    ScriptRegistrar m_registerScripts;
    NewSceneBuilder m_buildNewScene;
    std::vector<ScenePreset> m_scenePresets;
    AssetCatalog m_assetCatalog;
    EditorLog m_editorLog;
    EditorLogSink m_scriptLogSink;
    EditorPlayMode m_playmode;
    EditorTemplate m_template;
    Nebula::SceneSerializer m_sceneSerializer;
    int m_qtGizmoMode = 0;
    QtGizmoTool m_qtGizmo;
    float m_lastFrameDt = 0.016f;
    bool m_qtPendingPick = false;
    float m_qtClickX = 0.f;
    float m_qtClickY = 0.f;
    float m_qtPointerX = 0.f;
    float m_qtPointerY = 0.f;
    bool m_qtRightDragging = false;
    bool m_qtLeftLooking = false;
    ColliderHandle m_qtColliderHandle = ColliderHandle::None;
    bool m_qtColliderSymmetric = false;

    void enterPlayMode();
    void exitPlayMode();
    void deleteSelectedEntity();
    void saveSelectedEntityAsPrefab();
    void newScene(NewSceneBuilder builder);
    void newSceneFromPath(std::string_view path);
    bool loadSceneFromAbsolutePath(const std::string &absolutePath);
    void refreshAssetCatalog();
    void spawnMeshAt(std::string_view meshPath, std::string_view materialPath, const Nebula::Vec3 &position);
    void spawnPrefabAt(std::string_view prefabPath, const Nebula::Vec3 &position);
    Nebula::FileAssetProvider *fileAssets();
    void handleQtSceneInteraction();
  };
}
