#include "inspectorPanel.h"
#include "physics/physics_component.h"
#include <imgui.h>

#include <cstring>
#include <string>
#include <vector>

namespace Editor
{
  namespace
  {
    void applyBuiltinMeshPreset(Nebula::MeshRendererComponent &meshRenderer, int presetIndex)
    {
      if (presetIndex == 0)
      {
        meshRenderer.m_meshPath = "builtin/meshes/cube";
        meshRenderer.m_materialPath = "builtin/materials/cube";
      }

      else if (presetIndex == 1)
      {
        meshRenderer.m_meshPath = "builtin/meshes/ground";
        meshRenderer.m_materialPath = "builtin/materials/ground";
      }
      else if (presetIndex == 2)
      {
        meshRenderer.m_meshPath = "builtin/meshes/cube";
        meshRenderer.m_materialPath = "builtin/materials/enemy";
      }
      else if (presetIndex == 3)
      {
        meshRenderer.m_meshPath = "builtin/meshes/cube";
        meshRenderer.m_materialPath = "builtin/materials/player";
      }
    }

    int meshPresetIndexFromPaths(const Nebula::MeshRendererComponent &meshRenderer)
    {
      if (meshRenderer.m_materialPath == "builtin/materials/ground")
      {
        return 1;
      }
      else if (meshRenderer.m_materialPath == "builtin/materials/enemy")
      {
        return 2;
      }
      else if (meshRenderer.m_materialPath == "builtin/materials/player")
      {
        return 3;
      }
      return 0;
    }

    std::string defaultScriptId(const Nebula::ScriptRegistry &scriptRegistry)
    {
      const std::vector<std::string> ids = scriptRegistry.registeredScriptIds();
      if (ids.empty())
      {
        return {};
      }
      return ids.front();
    }
  } // namespace

  bool InspectorPanel::drawComponentHeaderWithRemove(const char *title, const char *buttonId)
  {
    ImGui::Separator();
    ImGui::TextUnformatted(title);
    ImGui::SameLine();
    return ImGui::SmallButton(buttonId);
  }

  void InspectorPanel::drawMeshRendererFields(Nebula::MeshRendererComponent &meshRenderer,
                                              EditorState &state,
                                              const std::function<void()> &onSceneEdited)
  {
    int presetIndex = meshPresetIndexFromPaths(meshRenderer);
    const char *presetLabels[] = {"Cube", "Ground", "Enemy", "Player"};
    if (ImGui::Combo("Preset", &presetIndex, presetLabels, 4))
    {
      applyBuiltinMeshPreset(meshRenderer, presetIndex);
      state.sceneDirty = true;
      onSceneEdited();
    }

    char meshPathBuf[256];
    std::strncpy(meshPathBuf, meshRenderer.m_meshPath.c_str(), sizeof(meshPathBuf) - 1);
    meshPathBuf[sizeof(meshPathBuf) - 1] = '\0';
    if (ImGui::InputText("Mesh Path", meshPathBuf, sizeof(meshPathBuf)))
    {
      meshRenderer.m_meshPath = meshPathBuf;
      state.sceneDirty = true;
      onSceneEdited();
    }

    char materialPathBuf[256];
    std::strncpy(materialPathBuf, meshRenderer.m_materialPath.c_str(), sizeof(materialPathBuf) - 1);
    materialPathBuf[sizeof(materialPathBuf) - 1] = '\0';
    if (ImGui::InputText("Material Path", materialPathBuf, sizeof(materialPathBuf)))
    {
      meshRenderer.m_materialPath = materialPathBuf;
      state.sceneDirty = true;
      onSceneEdited();
    }
  }

  void InspectorPanel::drawCameraFields(Nebula::CameraComponent &camera, EditorState &state)
  {
    if (ImGui::DragFloat("Distance", &camera.distance, 0.05f, 0.5f, 64.f))
    {
      state.sceneDirty = true;
    }
    float pivot[3] = {camera.pivotOffset.x, camera.pivotOffset.y, camera.pivotOffset.z};
    if (ImGui::DragFloat3("Pivot Offset", pivot, 0.01f))
    {
      camera.pivotOffset = {pivot[0], pivot[1], pivot[2]};
      state.sceneDirty = true;
    }
    if (ImGui::DragFloat("Pitch", &camera.pitch, 0.01f, -1.5f, 1.5f))
    {
      state.sceneDirty = true;
    }
    if (ImGui::DragFloat("Yaw", &camera.yaw, 0.01f))
    {
      state.sceneDirty = true;
    }
    char targetTagBuf[128];
    std::strncpy(targetTagBuf, camera.targetTag.c_str(), sizeof(targetTagBuf) - 1);
    targetTagBuf[sizeof(targetTagBuf) - 1] = '\0';
    if (ImGui::InputText("Target Tag", targetTagBuf, sizeof(targetTagBuf)))
    {
      camera.targetTag = targetTagBuf;
      state.sceneDirty = true;
    }
  }

  void InspectorPanel::drawColliderFields(Nebula::ColliderComponent &collider, EditorState &state)
  {
    int shapeIndex = collider.shape == Nebula::ColliderComponent::Shape::Sphere ? 1 : 0;
    const char *shapeLabels[] = {"Box", "Sphere"};

    if (ImGui::Combo("Shape", &shapeIndex, shapeLabels, 2))
    {
      collider.shape = shapeIndex == 1
                           ? Nebula::ColliderComponent::Shape::Sphere
                           : Nebula::ColliderComponent::Shape::Box;
      state.sceneDirty = true;
    }

    float halfExtents[3] = {collider.halfExtents.x, collider.halfExtents.y, collider.halfExtents.z};

    if (ImGui::DragFloat3("Half Extents", halfExtents, 0.01f, 0.01f, 100.f))
    {
      collider.halfExtents = {halfExtents[0], halfExtents[1], halfExtents[2]};
      state.sceneDirty = true;
    }

    if (ImGui::Checkbox("Is Trigger", &collider.isTrigger))
    {
      state.sceneDirty = true;
    }

    if (ImGui::Checkbox("Is Static", &collider.isStatic))
    {
      state.sceneDirty = true;
    }
  }

  void InspectorPanel::drawRigidBodyFields(Nebula::RigidBodyComponent &body, EditorState &state)
  {
    if (ImGui::Checkbox("Kinematic", &body.kinematic))
      state.sceneDirty = true;
    if (ImGui::DragFloat("Mass", &body.mass, 0.1f, 0.01f, 1000.f))
      state.sceneDirty = true;
  }

  void InspectorPanel::drawScriptSelector(Nebula::ScriptComponent &script,
                                          Nebula::ScriptFieldRegistry &fieldRegistry,
                                          Nebula::ScriptRegistry &scriptRegistry, EditorState &state)
  {
    const std::string preview = script.scriptName.empty() ? "(none)" : script.scriptName;
    if (ImGui::BeginCombo("Type", preview.c_str()))
    {
      for (const std::string &id : scriptRegistry.registeredScriptIds())
      {
        const bool selected = id == script.scriptName;
        if (ImGui::Selectable(id.c_str(), selected))
        {
          script.scriptName = id;
          script.paramsJson =
              m_scriptParams.mergeScriptParamDefaults(script.paramsJson, fieldRegistry, script.scriptName);
          state.sceneDirty = true;
        }
        if (selected)
        {
          ImGui::SetItemDefaultFocus();
        }
      }
      ImGui::EndCombo();
    }
  }

  void InspectorPanel::drawInspectorPanel(EditorState &state, Nebula::Scene &scene,
                                          Nebula::Entity entity, Nebula::ScriptFieldRegistry &scriptFieldRegistry,
                                          Nebula::ScriptRegistry &scriptRegistry, std::function<void()> onSceneEdited)
  {
    ImGui::Begin("Inspector");

    if (!scene.isValidEntity(entity))
    {
      ImGui::TextUnformatted("No entity selected.");
      ImGui::End();
      return;
    }

    if (!scene.hasComponent<Nebula::TransformComponent>(entity))
    {
      scene.addComponent<Nebula::TransformComponent>(entity);
    }

    if (scene.hasComponent<Nebula::TransformComponent>(entity))
    {
      auto &t = scene.getComponent<Nebula::TransformComponent>(entity).transform;
      float pos[3] = {t.getPosition().x, t.getPosition().y, t.getPosition().z};
      if (ImGui::DragFloat3("Position", pos, 0.05f))
      {
        t.setPosition({pos[0], pos[1], pos[2]});
        state.sceneDirty = true;
      }
      float yaw = t.getYaw();
      if (ImGui::DragFloat("Yaw", &yaw, 0.01f))
      {
        t.setYaw(yaw);
        state.sceneDirty = true;
      }
      float scale = t.getScale();
      if (ImGui::DragFloat("Scale", &scale, 0.01f))
      {
        t.setScale(scale);
        state.sceneDirty = true;
      }
    }

    if (scene.hasComponent<Nebula::TagComponent>(entity))
    {
      if (drawComponentHeaderWithRemove("Tag", "Remove##Tag"))
      {
        scene.removeComponent<Nebula::TagComponent>(entity);
        state.sceneDirty = true;
      }
      else
      {
        auto &tag = scene.getComponent<Nebula::TagComponent>(entity);
        char buf[128];
        std::strncpy(buf, tag.tag.c_str(), sizeof(buf) - 1);
        buf[sizeof(buf) - 1] = '\0';

        if (ImGui::InputText("Name", buf, sizeof(buf)))
        {
          tag.tag = buf;
          state.sceneDirty = true;
        }
      }
    }
    else if (ImGui::Button("Add Tag"))
    {
      scene.addComponent<Nebula::TagComponent>(entity).tag = "Entity";
      state.sceneDirty = true;
    }

    if (scene.hasComponent<Nebula::MeshRendererComponent>(entity))
    {
      if (drawComponentHeaderWithRemove("Mesh Renderer", "Remove##MeshRenderer"))
      {
        scene.removeComponent<Nebula::MeshRendererComponent>(entity);
        state.sceneDirty = true;
        onSceneEdited();
      }
      else
      {
        drawMeshRendererFields(scene.getComponent<Nebula::MeshRendererComponent>(entity), state, onSceneEdited);
      }
    }

    if (scene.hasComponent<Nebula::ScriptComponent>(entity))
    {
      if (drawComponentHeaderWithRemove("Script", "Remove##Script"))
      {
        scene.removeComponent<Nebula::ScriptComponent>(entity);
        state.sceneDirty = true;
      }
      else
      {
        drawScriptSelector(scene.getComponent<Nebula::ScriptComponent>(entity), scriptFieldRegistry, scriptRegistry,
                           state);
        drawScriptFields(entity, scriptFieldRegistry, scene, state);
      }
    }

    if (scene.hasComponent<Nebula::CameraComponent>(entity))
    {
      if (drawComponentHeaderWithRemove("Camera", "Remove##Camera"))
      {
        scene.removeComponent<Nebula::CameraComponent>(entity);
        state.sceneDirty = true;
      }
      else
      {
        drawCameraFields(scene.getComponent<Nebula::CameraComponent>(entity), state);
      }
    }
    if (scene.hasComponent<Nebula::ColliderComponent>(entity))
    {
      if (drawComponentHeaderWithRemove("Collider", "Remove##Collider"))
      {
        scene.removeComponent<Nebula::ColliderComponent>(entity);
        state.sceneDirty = true;
      }
      else
      {
        drawColliderFields(scene.getComponent<Nebula::ColliderComponent>(entity), state);
      }
    }
    if (scene.hasComponent<Nebula::RigidBodyComponent>(entity))
    {
      if (drawComponentHeaderWithRemove("Rigid Body", "Remove##RigidBody"))
      {
        scene.removeComponent<Nebula::RigidBodyComponent>(entity);
        state.sceneDirty = true;
      }
      else
      {
        drawRigidBodyFields(scene.getComponent<Nebula::RigidBodyComponent>(entity), state);
      }
    }

    if (ImGui::Button("Add Component"))
    {
      ImGui::OpenPopup("AddComponentPopup");
    }
    if (ImGui::BeginPopup("AddComponentPopup"))
    {
      if (!scene.hasComponent<Nebula::MeshRendererComponent>(entity) && ImGui::MenuItem("Mesh Renderer"))
      {
        auto &meshRenderer = scene.addComponent<Nebula::MeshRendererComponent>(entity);
        applyBuiltinMeshPreset(meshRenderer, 0);
        state.sceneDirty = true;
        onSceneEdited();
      }
      if (!scene.hasComponent<Nebula::TagComponent>(entity) && ImGui::MenuItem("Tag"))
      {
        scene.addComponent<Nebula::TagComponent>(entity).tag = "Entity";
        state.sceneDirty = true;
      }
      if (!scene.hasComponent<Nebula::ScriptComponent>(entity) && ImGui::MenuItem("Script"))
      {
        auto &sc = scene.addComponent<Nebula::ScriptComponent>(entity);
        sc.scriptName = defaultScriptId(scriptRegistry);
        sc.paramsJson = "{}";
        if (!sc.scriptName.empty())
        {
          sc.paramsJson =
              m_scriptParams.mergeScriptParamDefaults(sc.paramsJson, scriptFieldRegistry, sc.scriptName);
        }
        state.sceneDirty = true;
      }
      if (!scene.hasComponent<Nebula::CameraComponent>(entity) && ImGui::MenuItem("Camera"))
      {
        scene.addComponent<Nebula::CameraComponent>(entity);
        state.sceneDirty = true;
      }
      if (!scene.hasComponent<Nebula::ColliderComponent>(entity) && ImGui::MenuItem("Collider"))
      {
        scene.addComponent<Nebula::ColliderComponent>(entity);
        state.sceneDirty = true;
      }
      if (!scene.hasComponent<Nebula::RigidBodyComponent>(entity) && ImGui::MenuItem("Rigid Body"))
      {
        scene.addComponent<Nebula::RigidBodyComponent>(entity);
        state.sceneDirty = true;
      }
      ImGui::EndPopup();
    }

    ImGui::End();
  }

  void InspectorPanel::drawScriptFields(Nebula::Entity entity, Nebula::ScriptFieldRegistry &fieldRegistry,
                                        Nebula::Scene &scene, EditorState &state)
  {
    if (!scene.hasComponent<Nebula::ScriptComponent>(entity))
    {
      return;
    }

    auto &script = scene.getComponent<Nebula::ScriptComponent>(entity);
    if (script.scriptName.empty())
    {
      return;
    }

    if (const auto *fields = fieldRegistry.getFields(script.scriptName))
    {
      script.paramsJson = m_scriptParams.mergeScriptParamDefaults(script.paramsJson, fieldRegistry, script.scriptName);
      for (const auto &field : *fields)
      {
        if (!field.visibleInEditor)
        {
          continue;
        }
        switch (field.type)
        {
        case Nebula::ScriptFieldType::Float:
        {
          float v = m_scriptParams.readScriptParamFloat(script.paramsJson, field);
          if (ImGui::DragFloat(field.name.c_str(), &v, 0.01f))
          {
            script.paramsJson = m_scriptParams.setScriptParamFloat(script.paramsJson, field.name, v);
            state.sceneDirty = true;
          }
          break;
        }
        case Nebula::ScriptFieldType::Int:
        {
          int v = m_scriptParams.readScriptParamInt(script.paramsJson, field);
          if (ImGui::DragInt(field.name.c_str(), &v))
          {
            script.paramsJson = m_scriptParams.setScriptParamInt(script.paramsJson, field.name, v);
            state.sceneDirty = true;
          }
          break;
        }
        case Nebula::ScriptFieldType::Bool:
        {
          bool v = m_scriptParams.readScriptParamBool(script.paramsJson, field);
          if (ImGui::Checkbox(field.name.c_str(), &v))
          {
            script.paramsJson = m_scriptParams.setScriptParamBool(script.paramsJson, field.name, v);
            state.sceneDirty = true;
          }
          break;
        }
        }
      }
    }
  }
} // namespace Editor
