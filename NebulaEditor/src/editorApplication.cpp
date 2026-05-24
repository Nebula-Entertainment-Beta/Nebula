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
      m_registerScripts(getScriptRegistry(), getScriptFieldRegistry());
    }

    Nebula::Scene &scene = getScene();
    if (!Nebula::SceneSerializer::load(scene, getAssets(), m_state.scenePath))
    {
      scene.clear();
    }
    resolveSceneAssets();
    Nebula::Application::onStartup();
    NebulaEditor::ImGuiLayer::init(getWindow());
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
    ImGui::Begin("Hierarchy");
    ImGui::TextUnformatted("Entity list (Task 4+)");
    ImGui::End();

    ImGui::Begin("Console");
    ImGui::TextUnformatted("Log output (Task 4+)");
    ImGui::End();

    ImGui::Begin("Inspector");
    if (m_state.selectedEntity.id != 0)
    {
      ImGui::Text("Selected entity id: %u", m_state.selectedEntity.id);
    }
    else
    {
      ImGui::TextUnformatted("No entity selected");
    }
    ImGui::End();

    ImGui::Begin("Scene View");
    ImGui::TextUnformatted("Scene viewport (Task 4+)");
    ImGui::End();
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
        }
      }
      else if (ImGui::MenuItem("Stop"))
      {
        setPlaying(false);
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
    }
  }

  void EditorApplication::openSceneDialog()
  {
    Nebula::Scene &scene = getScene();
    const bool loaded = Nebula::SceneSerializer::load(scene, getAssets(), m_state.scenePath);

    if (!loaded)
    {
      return;
    }

    m_state.selectedEntity = {};
    m_state.sceneDirty = false;

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
