#include "inspectorPanel.h"
#include <imgui.h>

namespace Editor
{

  void InspectorPanel::drawInspectorPanel(EditorState &state, Nebula::Scene &scene, Nebula::Entity entity, Nebula::ScriptFieldRegistry &scriptFieldRegistry)
  {
    ImGui::Begin("Inspector");

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
    drawScriptFields(entity, scriptFieldRegistry, scene, state);

    ImGui::End();
  }

  void InspectorPanel::drawScriptFields(Nebula::Entity entity, Nebula::ScriptFieldRegistry &fieldRegistry, Nebula::Scene &scene, EditorState &state)
  {
    if (scene.hasComponent<Nebula::ScriptComponent>(entity))
    {
      auto &script = scene.getComponent<Nebula::ScriptComponent>(entity);
      if (const auto *fields = fieldRegistry.getFields(script.scriptName))
      {
        script.paramsJson = m_scriptParams.mergeScriptParamDefaults(script.paramsJson, fieldRegistry, script.scriptName);
        for (const auto &field : *fields)
        {
          if (!field.visibleInEditor)
            continue;
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
  }
}