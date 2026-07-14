#include "inspectorPanel.h"
#include "collider_fit.h"
#include "physics/physics_component.h"
#include "prefabInstance.h"
#include "prefabService.h"

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

    std::string entityVectorToJson(const std::vector<Nebula::Entity> &entities)
    {
      std::string json = "[";
      for (std::size_t i = 0; i < entities.size(); ++i)
      {
        if (i > 0)
        {
          json += ",";
        }
        json += "{\"id\":" + std::to_string(entities[i].id) +
                ",\"generation\":" + std::to_string(entities[i].generation) + "}";
      }
      json += "]";
      return json;
    }
  } // namespace

  bool InspectorPanel::drawComponentHeaderWithRemove(const char *title, const char *buttonId)
  {
    ImGui::Separator();
    ImGui::TextUnformatted(title);
    ImGui::SameLine();
    return ImGui::SmallButton(buttonId);
  }

  std::string InspectorPanel::entityLabel(Nebula::Scene &scene, Nebula::Entity entity) const
  {
    if (!scene.isValidEntity(entity))
    {
      return "(invalid)";
    }
    if (scene.hasComponent<Nebula::TagComponent>(entity))
    {
      return scene.getComponent<Nebula::TagComponent>(entity).tag;
    }
    return "Entity " + std::to_string(entity.id);
  }

  bool InspectorPanel::isOverrideActive(Nebula::Scene &scene, Nebula::Entity entity, const char *componentKey,
                                        const char *fieldKey) const
  {
    if (!scene.hasComponent<Nebula::PrefabInstanceComponent>(entity))
    {
      return false;
    }
    const auto &inst = scene.getComponent<Nebula::PrefabInstanceComponent>(entity);
    return Nebula::PrefabService::overridesContainPath(inst.overridesJson, componentKey, fieldKey);
  }

  void InspectorPanel::drawPrefabInstanceHeader(Nebula::Scene &scene, Nebula::Entity entity, EditorState &state,
                                                std::function<void()> onRevertPrefab,
                                                std::function<void()> onCreateVariant)
  {
    if (!scene.hasComponent<Nebula::PrefabInstanceComponent>(entity))
    {
      return;
    }

    const auto &inst = scene.getComponent<Nebula::PrefabInstanceComponent>(entity);
    ImGui::Separator();
    ImGui::TextUnformatted("Prefab Instance");
    ImGui::Text("Source: %s", inst.prefabPath.c_str());
    if (ImGui::Button("Revert to Prefab"))
    {
      onRevertPrefab();
    }
    ImGui::SameLine();
    if (ImGui::Button("Create Variant"))
    {
      onCreateVariant();
      state.sceneDirty = true;
    }
  }

  void InspectorPanel::drawOverrideResetButton(Nebula::Scene &scene, Nebula::Entity entity, EditorState &state,
                                               Nebula::AssetManager &assets, Nebula::IAssetProvider &fileProvider,
                                               const char *componentKey, const char *fieldKey)
  {
    if (!isOverrideActive(scene, entity, componentKey, fieldKey))
    {
      return;
    }
    ImGui::SameLine();
    if (ImGui::SmallButton("Reset"))
    {
      if (Nebula::PrefabService::resetInstanceOverrideField(scene, entity, assets, fileProvider, componentKey,
                                                            fieldKey))
      {
        state.sceneDirty = true;
      }
    }
  }

  void InspectorPanel::drawTransformFields(Nebula::Scene &scene, Nebula::Entity entity, EditorState &state,
                                         bool isPrefabInstance, Nebula::AssetManager &assets,
                                         Nebula::IAssetProvider &fileProvider)
  {
    auto &t = scene.getComponent<Nebula::TransformComponent>(entity).transform;
    float pos[3] = {t.getPosition().x, t.getPosition().y, t.getPosition().z};

    std::string posLabel = "Position";
    if (isPrefabInstance && isOverrideActive(scene, entity, "TransformComponent", "position"))
    {
      posLabel += "*";
    }
    if (ImGui::DragFloat3(posLabel.c_str(), pos, 0.05f))
    {
      t.setPosition({pos[0], pos[1], pos[2]});
      if (isPrefabInstance)
      {
        auto &inst = scene.getComponent<Nebula::PrefabInstanceComponent>(entity);
        Nebula::PrefabService::setInstanceOverrideField(inst, "TransformComponent", "position", pos);
      }
      state.sceneDirty = true;
    }
    if (isPrefabInstance)
    {
      drawOverrideResetButton(scene, entity, state, assets, fileProvider, "TransformComponent", "position");
    }

    float yaw = t.getYaw();
    std::string yawLabel = "Yaw";
    if (isPrefabInstance && isOverrideActive(scene, entity, "TransformComponent", "yaw"))
    {
      yawLabel += "*";
    }
    if (ImGui::DragFloat(yawLabel.c_str(), &yaw, 0.01f))
    {
      t.setYaw(yaw);
      if (isPrefabInstance)
      {
        auto &inst = scene.getComponent<Nebula::PrefabInstanceComponent>(entity);
        Nebula::PrefabService::setInstanceOverrideField(inst, "TransformComponent", "yaw", yaw);
      }
      state.sceneDirty = true;
    }
    if (isPrefabInstance)
    {
      drawOverrideResetButton(scene, entity, state, assets, fileProvider, "TransformComponent", "yaw");
    }

    Nebula::Vec3 scale = t.getScale();
    std::string scaleLabel = "Scale";
    if (isPrefabInstance && isOverrideActive(scene, entity, "TransformComponent", "scale"))
    {
      scaleLabel += "*";
    }
    if (ImGui::DragFloat3(scaleLabel.c_str(), &scale.x, 0.01f))
    {
      t.setScale(scale);
      if (isPrefabInstance)
      {
        auto &inst = scene.getComponent<Nebula::PrefabInstanceComponent>(entity);
        const float scaleArr[3] = {scale.x, scale.y, scale.z};
        Nebula::PrefabService::setInstanceOverrideField(inst, "TransformComponent", "scale", scaleArr);
      }
      state.sceneDirty = true;
    }
    if (isPrefabInstance)
    {
      drawOverrideResetButton(scene, entity, state, assets, fileProvider, "TransformComponent", "scale");
    }
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

  void InspectorPanel::drawColliderFields(Nebula::ColliderComponent &collider, Nebula::Scene &scene,
                                          Nebula::Entity entity, Nebula::AssetManager &assets,
                                          EditorState &state, const std::function<void()> &onSceneEdited)
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

    if (scene.hasComponent<Nebula::MeshRendererComponent>(entity))
    {
      if (ImGui::Button("Fit to Mesh"))
      {
        auto &meshRenderer = scene.getComponent<Nebula::MeshRendererComponent>(entity);
        if (Nebula::fitBoxColliderToMeshRenderer(collider, assets, meshRenderer))
        {
          state.sceneDirty = true;
          onSceneEdited();
        }
      }
      ImGui::SameLine();
      ImGui::TextDisabled("(local size; transform scale applies in world)");
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

  void InspectorPanel::drawEntityVectorField(Nebula::Entity owner, const Nebula::ScriptFieldDescriptor &field,
                                             Nebula::ScriptComponent &script, Nebula::Scene &scene,
                                             EditorState &state, bool isPrefabInstance)
  {
    std::vector<Nebula::Entity> entities =
        m_scriptParams.readScriptParamEntityVector(script.paramsJson, field);

    std::string header = field.name;
    if (isPrefabInstance)
    {
      const std::string overrideKey = "paramsJson." + field.name;
      if (isOverrideActive(scene, owner, "ScriptComponent", overrideKey.c_str()))
      {
        header += "*";
      }
    }
    ImGui::TextUnformatted(header.c_str());

    int removeIndex = -1;
    for (int i = 0; i < static_cast<int>(entities.size()); ++i)
    {
      ImGui::PushID(i);
      const std::string currentLabel = entityLabel(scene, entities[static_cast<std::size_t>(i)]);
      if (ImGui::BeginCombo("Entity", currentLabel.c_str()))
      {
        for (Nebula::Entity candidate : scene.getAllEntities())
        {
          if (candidate == owner)
          {
            continue;
          }
          const std::string candidateLabel = entityLabel(scene, candidate);
          const bool selected = entities[static_cast<std::size_t>(i)] == candidate;
          if (ImGui::Selectable(candidateLabel.c_str(), selected))
          {
            entities[static_cast<std::size_t>(i)] = candidate;
            script.paramsJson = m_scriptParams.setScriptParamEntityVector(script.paramsJson, field.name, entities);
            if (isPrefabInstance)
            {
              auto &inst = scene.getComponent<Nebula::PrefabInstanceComponent>(owner);
              Nebula::PrefabService::setInstanceOverrideFieldJson(
                  inst, "ScriptComponent", ("paramsJson." + field.name).c_str(),
                  entityVectorToJson(entities));
            }
            state.sceneDirty = true;
          }
        }
        ImGui::EndCombo();
      }
      ImGui::SameLine();
      if (ImGui::SmallButton("Remove"))
      {
        removeIndex = i;
      }
      ImGui::PopID();
    }

    if (removeIndex >= 0)
    {
      entities.erase(entities.begin() + removeIndex);
      script.paramsJson = m_scriptParams.setScriptParamEntityVector(script.paramsJson, field.name, entities);
      if (isPrefabInstance)
      {
        auto &inst = scene.getComponent<Nebula::PrefabInstanceComponent>(owner);
        Nebula::PrefabService::setInstanceOverrideFieldJson(
            inst, "ScriptComponent", ("paramsJson." + field.name).c_str(), entityVectorToJson(entities));
      }
      state.sceneDirty = true;
    }

    if (ImGui::Button("Add Entity"))
    {
      for (Nebula::Entity candidate : scene.getAllEntities())
      {
        if (candidate == owner)
        {
          continue;
        }
        entities.push_back(candidate);
        break;
      }
      script.paramsJson = m_scriptParams.setScriptParamEntityVector(script.paramsJson, field.name, entities);
      if (isPrefabInstance)
      {
        auto &inst = scene.getComponent<Nebula::PrefabInstanceComponent>(owner);
        Nebula::PrefabService::setInstanceOverrideFieldJson(
            inst, "ScriptComponent", ("paramsJson." + field.name).c_str(), entityVectorToJson(entities));
      }
      state.sceneDirty = true;
    }
  }

  void InspectorPanel::drawInspectorPanel(EditorState &state, Nebula::Scene &scene,
                                          Nebula::Entity entity, Nebula::ScriptFieldRegistry &scriptFieldRegistry,
                                          Nebula::ScriptRegistry &scriptRegistry, Nebula::AssetManager &assets,
                                          Nebula::IAssetProvider &fileProvider,
                                          Nebula::IRenderResourceFactory &renderResources,
                                          std::function<void()> onSceneEdited,
                                          std::function<void()> onRevertPrefab,
                                          std::function<void()> onCreateVariant)
  {
    (void)fileProvider;
    (void)renderResources;
    ImGui::Begin("Inspector");

    if (!scene.isValidEntity(entity))
    {
      ImGui::TextUnformatted("No entity selected.");
      ImGui::End();
      return;
    }

    const bool isPrefabInstance = scene.hasComponent<Nebula::PrefabInstanceComponent>(entity);
    drawPrefabInstanceHeader(scene, entity, state, onRevertPrefab, onCreateVariant);

    if (!scene.hasComponent<Nebula::TransformComponent>(entity))
    {
      scene.addComponent<Nebula::TransformComponent>(entity);
    }

    if (scene.hasComponent<Nebula::TransformComponent>(entity))
    {
      drawTransformFields(scene, entity, state, isPrefabInstance, assets, fileProvider);
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
        drawScriptFields(entity, scriptFieldRegistry, scene, state, isPrefabInstance, assets, fileProvider);
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
        drawColliderFields(scene.getComponent<Nebula::ColliderComponent>(entity), scene, entity, assets,
                           state, onSceneEdited);
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
                                        Nebula::Scene &scene, EditorState &state, bool isPrefabInstance,
                                        Nebula::AssetManager &assets, Nebula::IAssetProvider &fileProvider)
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
          std::string label = field.name;
          if (isPrefabInstance)
          {
            const std::string overrideKey = "paramsJson." + field.name;
            if (isOverrideActive(scene, entity, "ScriptComponent", overrideKey.c_str()))
            {
              label += "*";
            }
          }
          if (ImGui::DragFloat(label.c_str(), &v, 0.01f))
          {
            script.paramsJson = m_scriptParams.setScriptParamFloat(script.paramsJson, field.name, v);
            if (isPrefabInstance)
            {
              auto &inst = scene.getComponent<Nebula::PrefabInstanceComponent>(entity);
              Nebula::PrefabService::setInstanceOverrideField(
                  inst, "ScriptComponent", ("paramsJson." + field.name).c_str(), v);
            }
            state.sceneDirty = true;
          }
          if (isPrefabInstance)
          {
            drawOverrideResetButton(scene, entity, state, assets, fileProvider, "ScriptComponent",
                                    ("paramsJson." + field.name).c_str());
          }
          break;
        }
        case Nebula::ScriptFieldType::Int:
        {
          int v = m_scriptParams.readScriptParamInt(script.paramsJson, field);
          std::string label = field.name;
          if (isPrefabInstance)
          {
            const std::string overrideKey = "paramsJson." + field.name;
            if (isOverrideActive(scene, entity, "ScriptComponent", overrideKey.c_str()))
            {
              label += "*";
            }
          }
          if (ImGui::DragInt(label.c_str(), &v))
          {
            script.paramsJson = m_scriptParams.setScriptParamInt(script.paramsJson, field.name, v);
            if (isPrefabInstance)
            {
              auto &inst = scene.getComponent<Nebula::PrefabInstanceComponent>(entity);
              Nebula::PrefabService::setInstanceOverrideField(
                  inst, "ScriptComponent", ("paramsJson." + field.name).c_str(), v);
            }
            state.sceneDirty = true;
          }
          if (isPrefabInstance)
          {
            drawOverrideResetButton(scene, entity, state, assets, fileProvider, "ScriptComponent",
                                    ("paramsJson." + field.name).c_str());
          }
          break;
        }
        case Nebula::ScriptFieldType::Bool:
        {
          bool v = m_scriptParams.readScriptParamBool(script.paramsJson, field);
          std::string label = field.name;
          if (isPrefabInstance)
          {
            const std::string overrideKey = "paramsJson." + field.name;
            if (isOverrideActive(scene, entity, "ScriptComponent", overrideKey.c_str()))
            {
              label += "*";
            }
          }
          if (ImGui::Checkbox(label.c_str(), &v))
          {
            script.paramsJson = m_scriptParams.setScriptParamBool(script.paramsJson, field.name, v);
            if (isPrefabInstance)
            {
              auto &inst = scene.getComponent<Nebula::PrefabInstanceComponent>(entity);
              Nebula::PrefabService::setInstanceOverrideField(
                  inst, "ScriptComponent", ("paramsJson." + field.name).c_str(), v);
            }
            state.sceneDirty = true;
          }
          if (isPrefabInstance)
          {
            drawOverrideResetButton(scene, entity, state, assets, fileProvider, "ScriptComponent",
                                    ("paramsJson." + field.name).c_str());
          }
          break;
        }
        case Nebula::ScriptFieldType::EntityVector:
        {
          drawEntityVectorField(entity, field, script, scene, state, isPrefabInstance);
          break;
        }
        }
      }
    }
  }
} // namespace Editor
