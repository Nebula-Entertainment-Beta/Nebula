#include "editorApplication.h"

#include "imGuiLayer.h"
#include "sceneSerializer.h"

#include <imgui.h>
#include <imgui_internal.h>

namespace Editor
{
  EditorApplication::EditorApplication(const Nebula::ApplicationSpec &spec)
      : Nebula::Application(spec)
  {
  }

  EditorApplication::EditorApplication(const Nebula::ApplicationSpec &spec,
                                       ScriptRegistrar registerScripts)
      : Nebula::Application(spec),
        m_registerScripts(std::move(registerScripts))
  {
  }

  EditorApplication::~EditorApplication()
  {
    NebulaEditor::ImGuiLayer::shutdown();
  }

  void EditorApplication::onStartup()
  {
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
    m_inspector.drawInspectorPanel(m_state, getScene(), m_state.selectedEntity, getScriptFieldRegistry());
    m_sceneViewPanel.drawSceneViewPanel(m_state, m_sceneViewFrameBuffer, getScene(), getAssetManager(), getRenderer(), getWindow());
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
        if (ImGui::MenuItem("Play"))
        {
          setPlaying(true);
          m_editorLog.info("Play");
        }
      }
      else if (ImGui::MenuItem("Stop"))
      {
        setPlaying(false);
        m_editorLog.info("Stop");
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
        if (ImGui::MenuItem("New Scene"))
        {
          newScene();
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
        if (ImGui::MenuItem("Create Entity"))
        {
          createEmptyEntity();
        }
        ImGui::EndMenu();
      }
      drawPlayStopToolbar();
      ImGui::EndMainMenuBar();
    }
  }

  void EditorApplication::newScene()
  {
    getScene().clear();
    m_state.selectedEntity = {};
    m_state.sceneDirty = false;

    resolveSceneAssets();
    Nebula::Application::onStartup();
    m_editorLog.info("New scene created");
  }

  void EditorApplication::saveScene()
  {
    const bool ok = Nebula::SceneSerializer::save(
        getScene(),
        getAssetManager(),
        getAssets(),
        m_state.scenePath);

    if (ok)
    {
      m_state.sceneDirty = false;
      m_editorLog.info("Scene saved: " + m_state.scenePath);
    }
    else
    {
      m_editorLog.info("Failed to save scene: " + m_state.scenePath);
    }
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

  void EditorApplication::createEmptyEntity()
  {
    Nebula::Scene &scene = getScene();
    const Nebula::Entity entity = scene.createEntity();
    scene.addComponent<Nebula::TransformComponent>(entity);

    m_state.selectedEntity = entity;
    m_state.sceneDirty = true;
  }

} // namespace Editor
