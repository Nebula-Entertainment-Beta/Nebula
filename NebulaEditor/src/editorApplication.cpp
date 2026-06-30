#include "editorApplication.h"

#include "imGuiLayer.h"
#include "eventBus.h"
#include "eventTypes.h"
#include "sceneSerializer.h"
#include "systemScheduler.h"
#include "prefabSerializer.h"
#include "prefabService.h"

#include <cctype>
#include <imgui.h>
#include <imgui_internal.h>

namespace Editor
{
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
    m_inspector.drawInspectorPanel(m_state, getScene(), m_state.selectedEntity,
                                   getScriptFieldRegistry(), getScriptRegistry(), getAssetManager(), [this]()
                                   {
      resolveSceneAssets();
      m_state.sceneDirty = true; });

    // call drawer
    DebugPanelDrawer isPlaying;

    m_sceneViewPanel.drawSceneViewPanel(m_state, m_sceneViewFrameBuffer, getScene(), getAssetManager(),
                                        getRenderer(), getWindow());
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
    ImGui::DockBuilderDockWindow("Inspector", dockRight);
    ImGui::DockBuilderDockWindow("Console", dockBottom);
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
              newScene(preset.build);
            }
          }
          ImGui::EndMenu();
        }
        if (ImGui::MenuItem("Open Scene"))
        {
          openSceneDialog();
        }
        if (ImGui::MenuItem("Save Scene", "Ctrl+S"))
        {
          saveScene();
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
        if (ImGui::BeginMenu("Instantiate Prefab"))
        {
          static const char *kKnownPrefabs[] = {
              "prefabs/enemy.prefab",
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
    Nebula::Scene &scene = getScene();
    const bool loaded = Nebula::SceneSerializer::load(scene, getAssets(), m_state.scenePath);

    if (!loaded)
    {
      m_editorLog.info("Failed to load scene: " + m_state.scenePath);
      return;
    }

    m_state.selectedEntity = {};
    m_state.sceneDirty = false;
    m_editorLog.info("Scene loaded: " + m_state.scenePath);

    resolveSceneAssets();
    Nebula::Application::onStartup();
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
