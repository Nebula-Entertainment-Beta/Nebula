#include "editorApplication.h"

#include "imGuiLayer.h"
#include "eventBus.h"
#include "eventTypes.h"
#include "sceneSerializer.h"
#include "systemScheduler.h"
#include "prefabSerializer.h"
#include "prefabService.h"
#include "platform/fileDialog.h"
#include "assetProvider.h"

#include <cctype>
#include <filesystem>
#include <imgui.h>
#include <imgui_internal.h>

namespace Editor
{
  namespace
  {
    std::string toAssetsRelativeScenePath(const std::string &absolutePath)
    {
      const std::filesystem::path scenesMarker = std::filesystem::path("assets") / "scenes";
      const auto pos = absolutePath.find(scenesMarker.generic_string());
      if (pos != std::string::npos)
      {
        return std::filesystem::path(absolutePath.substr(pos + std::string("assets/").size()))
            .generic_string();
      }
      return std::filesystem::path(absolutePath).filename().generic_string();
    }
  }

  EditorApplication::EditorApplication(const Nebula::ApplicationSpec &spec)
      : Nebula::Application(spec)
  {
    m_hierarchy.setEntityActions(
        [this](const char *id)
        { createEntityFromTemplate(id); },
        [this]()
        { deleteSelectedEntity(); });
    m_hierarchy.setPrefabActions(
        [this](std::string_view path)
        { instantiatePrefab(path); },
        [this]()
        { saveSelectedEntityAsPrefab(); });
  }

  EditorApplication::EditorApplication(const Nebula::ApplicationSpec &spec,
                                       ScriptRegistrar registerScripts,
                                       NewSceneBuilder buildNewScene,
                                       std::vector<ScenePreset> scenePresets)
      : Nebula::Application(spec),
        m_registerScripts(std::move(registerScripts)),
        m_buildNewScene(std::move(buildNewScene)),
        m_scenePresets(std::move(scenePresets))
  {
    m_hierarchy.setEntityActions(
        [this](const char *id)
        { createEntityFromTemplate(id); },
        [this]()
        { deleteSelectedEntity(); });
    m_hierarchy.setPrefabActions(
        [this](std::string_view path)
        { instantiatePrefab(path); },
        [this]()
        { saveSelectedEntityAsPrefab(); });
  }

  EditorApplication::~EditorApplication()
  {
    NebulaEditor::ImGuiLayer::shutdown();
  }

  void EditorLogSink::info(std::string_view msg)
  {
    if (m_log != nullptr)
    {
      m_log->info(msg);
    }
  }

  void EditorApplication::onStartup()
  {
    m_scriptLogSink.setLog(&m_editorLog);
    setLogSink(&m_scriptLogSink);

    if (m_registerScripts)
    {
      m_editorLog.info("Registering scripts...");
      m_registerScripts(getScriptRegistry(), getScriptFieldRegistry());
    }

    Nebula::Scene &scene = getScene();
    if (!Nebula::SceneSerializer::load(scene, getAssets(), m_state.scenePath))
    {
      m_editorLog.info("Failed to load scene: " + m_state.scenePath);
      scene.clear();
    }
    m_editorLog.info("Scene loaded: " + m_state.scenePath);

    resolveSceneAssets();
    refreshAssetCatalog();
    Nebula::Application::onStartup();
    NebulaEditor::ImGuiLayer::init(getWindow());
    m_editorLog.info("Editor started");
  }

  void EditorApplication::onRender()
  {
    Nebula::Application::onRender();
    NebulaEditor::ImGuiLayer::beginFrame();
    drawDockspace();
    drawEditorPanels();
    NebulaEditor::ImGuiLayer::endFrame();
  }

  void EditorApplication::drawEditorPanels()
  {

    m_hierarchy.drawHierarchyPanel(getScene(), m_state);
    m_console.drawConsolePanel(m_editorLog);
    m_debug.drawDebugPanel(m_state, getScene(), getInput(), getActionMapping(), getScriptFieldRegistry(),
                           isPlaying());
    m_inspector.drawInspectorPanel(m_state, getScene(), m_state.selectedEntity,
                                   getScriptFieldRegistry(), getScriptRegistry(), getAssetManager(),
                                   getAssets(), getRenderer().resources(),
                                   [this]()
                                   {
                                     resolveSceneAssets();
                                     m_state.sceneDirty = true;
                                   },
                                   [this]()
                                   { revertSelectedPrefabInstance(); },
                                   [this]()
                                   { createVariantFromSelectedInstance(); });

    m_sceneViewPanel.drawSceneViewPanel(
        m_state, m_sceneViewFrameBuffer, getScene(), getAssetManager(), getRenderer(), getWindow(),
        getInput(), isPlaying(),
        [this](std::string_view meshPath, std::string_view materialPath, const Nebula::Vec3 &pos)
        { spawnMeshAt(meshPath, materialPath, pos); },
        [this](std::string_view prefabPath, const Nebula::Vec3 &pos)
        { spawnPrefabAt(prefabPath, pos); });

    m_assetBrowser.draw(
        m_assetCatalog, m_state, getScene(), getAssetManager(),
        [this](std::string_view meshPath, std::string_view materialPath)
        { spawnMeshAt(meshPath, materialPath, Nebula::Vec3{0.0f, 0.5f, 0.0f}); },
        [this](std::string_view prefabPath)
        { spawnPrefabAt(prefabPath, Nebula::Vec3{0.0f, 0.5f, 0.0f}); },
        [this]()
        { refreshAssetCatalog(); });
  }

  void EditorApplication::setupDefaultDockLayout(ImGuiID dockspaceId)
  {
    if (m_dockLayoutBuilt)
    {
      return;
    }
    m_dockLayoutBuilt = true;

    ImGuiViewport *viewport = ImGui::GetMainViewport();
    ImGui::DockBuilderRemoveNode(dockspaceId);
    ImGui::DockBuilderAddNode(dockspaceId, ImGuiDockNodeFlags_DockSpace);
    ImGui::DockBuilderSetNodeSize(dockspaceId, viewport->WorkSize);

    ImGuiID dockMain = dockspaceId;
    ImGuiID dockLeft = ImGui::DockBuilderSplitNode(dockMain, ImGuiDir_Left, 0.22f, nullptr, &dockMain);
    ImGuiID dockRight = ImGui::DockBuilderSplitNode(dockMain, ImGuiDir_Right, 0.26f, nullptr, &dockMain);
    ImGuiID dockBottom = ImGui::DockBuilderSplitNode(dockMain, ImGuiDir_Down, 0.28f, nullptr, &dockMain);

    ImGui::DockBuilderDockWindow("Hierarchy", dockLeft);
    ImGui::DockBuilderDockWindow("Assets", dockLeft);
    ImGui::DockBuilderDockWindow("Inspector", dockRight);
    ImGui::DockBuilderDockWindow("Console", dockBottom);
    ImGui::DockBuilderDockWindow("Debug", dockBottom);
    ImGui::DockBuilderDockWindow("Scene View", dockMain);
    ImGui::DockBuilderFinish(dockspaceId);
  }

  void EditorApplication::drawPlayStopToolbar()
  {
    if (ImGui::BeginMenu("Play"))
    {
      if (!isPlaying())
      {
        if (ImGui::Button("Play"))
          enterPlayMode();
      }
      else
      {
        if (ImGui::Button("Stop"))
          exitPlayMode();
      }
      ImGui::EndMenu();
    }
  }

  void EditorApplication::drawDockspace()
  {
    const ImGuiID dockspaceId =
        ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode);
    setupDefaultDockLayout(dockspaceId);

    if (ImGui::BeginMainMenuBar())
    {
      if (ImGui::BeginMenu("File"))
      {
        if (m_scenePresets.empty())
        {
          if (ImGui::MenuItem("New Scene"))
          {
            newScene();
          }
        }
        else if (ImGui::BeginMenu("New Scene"))
        {
          for (const ScenePreset &preset : m_scenePresets)
          {
            if (ImGui::MenuItem(preset.label))
            {
              if (preset.loadPath != nullptr)
              {
                newSceneFromPath(preset.loadPath);
              }
              else
              {
                newScene(preset.build);
              }
            }
          }
          ImGui::EndMenu();
        }
        if (ImGui::MenuItem("Open Scene"))
        {
          const std::string path = openFileDialog("Open Scene", {"json"});
          if (!path.empty())
          {
            loadSceneFromAbsolutePath(path);
          }
        }
        if (ImGui::MenuItem("Save Scene", "Ctrl+S"))
        {
          saveScene();
        }
        if (ImGui::MenuItem("Save Scene As"))
        {
          const std::string path = saveFileDialog("Save Scene As", "my_level.json", {"json"});
          if (!path.empty())
          {
            m_state.scenePath = toAssetsRelativeScenePath(path);
            saveScene();
          }
        }
        ImGui::EndMenu();
      }
      if (ImGui::BeginMenu("Edit"))
      {
        if (ImGui::MenuItem("Create Empty"))
        {
          createEntityFromTemplate("empty");
        }
        if (ImGui::MenuItem("Create Mesh Cube"))
        {
          createEntityFromTemplate("cube");
        }
        if (ImGui::MenuItem("Create Enemy Placeholder"))
        {
          createEntityFromTemplate("enemy");
        }
        if (ImGui::MenuItem("Create Platform"))
        {
          createEntityFromTemplate("platform");
        }
        if (ImGui::MenuItem("Create Bounce Pad"))
        {
          createEntityFromTemplate("bouncePad");
        }
        if (ImGui::MenuItem("Create Wind Volume"))
        {
          createEntityFromTemplate("windVolume");
        }
        ImGui::Separator();
        if (ImGui::MenuItem("Save Selected as Prefab"))
        {
          saveSelectedEntityAsPrefab();
        }
        if (ImGui::MenuItem("Revert Selected to Prefab"))
        {
          revertSelectedPrefabInstance();
        }
        if (ImGui::MenuItem("Create Variant from Selected"))
        {
          createVariantFromSelectedInstance();
        }
        if (ImGui::BeginMenu("Instantiate Prefab"))
        {
          static const char *kKnownPrefabs[] = {
              "prefabs/enemy.prefab",
              "prefabs/enemy_fast.prefab",
              "prefabs/platform.prefab",
              "prefabs/bounce_pad.prefab",
              "prefabs/wind_volume.prefab",
          };
          for (const char *path : kKnownPrefabs)
          {
            if (ImGui::MenuItem(path))
            {
              instantiatePrefab(path);
            }
          }
          ImGui::EndMenu();
        }
        ImGui::EndMenu();
      }
      drawPlayStopToolbar();
      ImGui::EndMainMenuBar();
    }
  }

  void EditorApplication::enterPlayMode()
  {
    m_sceneSerializer.saveToString(m_playmode.snapshot(), getScene(), getAssetManager());
    setPlaying(true);
    rebuildScripts();
    m_editorLog.info("Play");
  }

  void EditorApplication::exitPlayMode()
  {
    setPlaying(false);
    Nebula::ScriptContext ctx = makeScriptContext();
    getScriptSystem().shutdownAll(ctx);
    m_sceneSerializer.loadFromString(getScene(), m_playmode.snapshot(), getAssets());
    resolveSceneAssets();
    rebuildScripts();
    m_editorLog.info("Stop");
  }

  void EditorApplication::openSceneDialog()
  {
    const std::string path = openFileDialog("Open Scene", {"json"});
    if (!path.empty())
    {
      loadSceneFromAbsolutePath(path);
    }
  }

  void EditorApplication::loadSceneFromAbsolutePath(const std::string &absolutePath)
  {
    Nebula::FileAssetProvider *files = fileAssets();
    if (files == nullptr)
    {
      return;
    }

    std::string logicalPath = absolutePath;
    for (const auto &root : files->searchRoots())
    {
      std::error_code ec;
      const auto rel = std::filesystem::relative(absolutePath, root, ec);
      if (!ec)
      {
        logicalPath = rel.generic_string();
        break;
      }
    }

    if (!loadScene(logicalPath))
    {
      m_editorLog.info("Failed to load scene: " + logicalPath);
      return;
    }
    resolveSceneAssets();
    Nebula::Application::onStartup();
    m_editorLog.info("Scene loaded: " + logicalPath);
  }

  Nebula::FileAssetProvider *EditorApplication::fileAssets()
  {
    return dynamic_cast<Nebula::FileAssetProvider *>(&getAssets());
  }

  void EditorApplication::refreshAssetCatalog()
  {
    if (Nebula::FileAssetProvider *files = fileAssets())
    {
      m_assetCatalog.refresh(*files);
    }
  }

  void EditorApplication::spawnMeshAt(std::string_view meshPath, std::string_view materialPath,
                                      const Nebula::Vec3 &position)
  {
    Nebula::Entity entity =
        m_template.createStaticMesh(getScene(), getAssetManager(), position, meshPath, materialPath);
    m_state.selectedEntity = entity;
    m_state.sceneDirty = true;
    resolveSceneAssets();
  }

  void EditorApplication::spawnPrefabAt(std::string_view prefabPath, const Nebula::Vec3 &position)
  {
    Nebula::Entity entity = instantiatePrefab(prefabPath);
    if (entity.id != 0 && getScene().hasComponent<Nebula::TransformComponent>(entity))
    {
      getScene().getComponent<Nebula::TransformComponent>(entity).transform.setPosition(position);
      m_state.sceneDirty = true;
    }
  }

  void EditorApplication::newScene()
  {
    if (m_buildNewScene)
    {
      newScene(m_buildNewScene);
      return;
    }

    getScene().clear();
    m_state.selectedEntity = {};
    m_state.sceneDirty = true;
    resolveSceneAssets();
    Nebula::Application::onStartup();
    m_editorLog.info("New scene created");
  }

  void EditorApplication::newScene(NewSceneBuilder builder)
  {
    getScene().clear();
    m_state.selectedEntity = {};

    if (builder)
    {
      builder(getScene());
    }

    m_state.sceneDirty = true;
    resolveSceneAssets();
    Nebula::Application::onStartup();

    m_editorLog.info("New scene created");
  }

  void EditorApplication::newSceneFromPath(std::string_view path)
  {
    getScene().clear();
    m_state.selectedEntity = {};

    if (!Nebula::SceneSerializer::load(getScene(), getAssets(), path))
    {
      m_editorLog.info("Failed to load scene: " + std::string(path));
      return;
    }

    m_state.scenePath = std::string(path);
    m_state.sceneDirty = false;
    resolveSceneAssets();
    Nebula::Application::onStartup();
    m_editorLog.info("Scene loaded: " + std::string(path));
  }

  void EditorApplication::createEmptyEntity()
  {
    createEntityFromTemplate("empty");
  }

  bool EditorApplication::saveSelectedAsPrefab(std::string_view path)
  {
    if (!getScene().isValidEntity(m_state.selectedEntity))
    {
      return false;
    }
    if (!Nebula::PrefabSerializer::save(getScene(), m_state.selectedEntity,
                                        getAssetManager(), getAssets(), path))
    {
      return false;
    }
    m_editorLog.info("Saved prefab: " + std::string(path));
    return true;
  }

  void EditorApplication::saveSelectedEntityAsPrefab()
  {
    Nebula::Scene &scene = getScene();
    if (!scene.isValidEntity(m_state.selectedEntity))
    {
      return;
    }

    std::string path = "prefabs/";
    if (scene.hasComponent<Nebula::TagComponent>(m_state.selectedEntity))
    {
      for (char c : scene.getComponent<Nebula::TagComponent>(m_state.selectedEntity).tag)
      {
        path.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(c))));
      }
    }
    else
    {
      path += "entity_" + std::to_string(m_state.selectedEntity.id);
    }
    path += ".prefab";

    if (saveSelectedAsPrefab(path))
    {
      m_state.sceneDirty = true;
    }
  }

  Nebula::Entity EditorApplication::instantiatePrefab(std::string_view path)
  {
    Nebula::Entity e = Nebula::PrefabService::instantiate(
        getScene(), getAssetManager(), getAssets(), getRenderer().resources(), path);
    if (e.id != 0)
    {
      m_state.selectedEntity = e;
      m_state.sceneDirty = true;
      resolveSceneAssets();
    }
    return e;
  }

  void EditorApplication::revertSelectedPrefabInstance()
  {
    if (!Nebula::PrefabService::revertInstance(getScene(), m_state.selectedEntity, getAssetManager(),
                                               getAssets(), getRenderer().resources()))
    {
      return;
    }
    m_state.sceneDirty = true;
    resolveSceneAssets();
    m_editorLog.info("Reverted prefab instance");
  }

  void EditorApplication::createVariantFromSelectedInstance()
  {
    Nebula::Scene &scene = getScene();
    if (!scene.isValidEntity(m_state.selectedEntity) ||
        !scene.hasComponent<Nebula::PrefabInstanceComponent>(m_state.selectedEntity))
    {
      return;
    }

    std::string path = "prefabs/";
    if (scene.hasComponent<Nebula::TagComponent>(m_state.selectedEntity))
    {
      for (char c : scene.getComponent<Nebula::TagComponent>(m_state.selectedEntity).tag)
      {
        path.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(c))));
      }
    }
    else
    {
      path += "entity_" + std::to_string(m_state.selectedEntity.id);
    }
    path += "_variant.prefab";

    if (Nebula::PrefabService::saveVariantFromInstance(scene, m_state.selectedEntity, getAssetManager(),
                                                        getAssets(), path))
    {
      m_editorLog.info("Saved prefab variant: " + path);
    }
  }

  void EditorApplication::deleteSelectedEntity()
  {
    if (m_state.selectedEntity == Nebula::Entity())
    {
      return;
    }
    Nebula::Scene &scene = getScene();
    if (!scene.isValidEntity(m_state.selectedEntity))
    {
      m_state.selectedEntity = {};
      return;
    }
    scene.destroyEntity(m_state.selectedEntity);
    m_state.selectedEntity = {};
    m_state.sceneDirty = true;
  }

  bool EditorApplication::saveScene()
  {
    if (!Nebula::SceneSerializer::save(getScene(), getAssetManager(), getAssets(), m_state.scenePath))
      return false;
    m_state.sceneDirty = false;
    getEventBus().push(Nebula::SceneSavedEvent{m_state.scenePath});
    m_editorLog.info("Saved " + m_state.scenePath);
    return true;
  }

  bool EditorApplication::loadScene(std::string_view path)
  {
    getScene().clear();
    if (!Nebula::SceneSerializer::load(getScene(), getAssets(), path))
      return false;
    getAssetManager().resolveScene(getScene(), getRenderer().resources());
    m_state.scenePath = std::string(path);
    m_state.selectedEntity = {};
    m_state.sceneDirty = false;
    return true;
  }

  void EditorApplication::registerGameSystems()
  {
    getScheduler().add(Nebula::SystemPhase::PreUpdate, [this](float)
                       {
                         for (const Nebula::GameEvent &event : getEventBus().events())
                         {
                           if (std::holds_alternative<Nebula::SaveSceneRequestedEvent>(event))
                           {
                             saveScene();
                             break;
                           }
                         } });
  }

  void EditorApplication::createEntityFromTemplate(const char *id)
  {
    Nebula::Scene &scene = getScene();
    Nebula::Entity e{};

    static const std::unordered_map<std::string, const char *> kPrefabForTemplate = {
        {"enemy", "prefabs/enemy.prefab"},
        {"platform", "prefabs/platform.prefab"},
        {"bouncePad", "prefabs/bounce_pad.prefab"},
        {"windVolume", "prefabs/wind_volume.prefab"},
    };
    if (auto it = kPrefabForTemplate.find(id); it != kPrefabForTemplate.end())
    {
      instantiatePrefab(it->second);
      return;
    }
    if (strcmp(id, "cube") == 0)
      e = m_template.createMeshCube(scene);

    else
      e = m_template.createEmpty(scene); // must add Transform inside
    m_state.selectedEntity = e;
    m_state.sceneDirty = true;
    resolveSceneAssets();
  }

} // namespace Editor
