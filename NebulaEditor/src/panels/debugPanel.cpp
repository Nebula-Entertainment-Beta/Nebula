#include "debugPanel.h"

#include "component.h"
#include "input_Actions.h"
#include "inputTypes.h"
#include "tag_component.h"

#include <imgui.h>

#include <cstring>

namespace Editor
{
  namespace
  {
    const char *tastoName(Nebula::Tasto key)
    {
      switch (key)
      {
      case Nebula::Tasto::w:
        return "W";
      case Nebula::Tasto::a:
        return "A";
      case Nebula::Tasto::s:
        return "S";
      case Nebula::Tasto::d:
        return "D";
      case Nebula::Tasto::space:
        return "Space";
      case Nebula::Tasto::left_shift:
        return "Shift";
      default:
        return "?";
      }
    }

    void drawKeyState(Nebula::Input &input, Nebula::Tasto key)
    {
      const bool down = input.isKeyDown(key);
      const bool pressed = input.wasKeyPressed(key);
      const bool released = input.wasKeyReleased(key);
      ImGui::Text("%s: %s%s%s", tastoName(key), down ? "Held " : "", pressed ? "Pressed " : "",
                  released ? "Released" : "");
    }

    void drawActionState(Nebula::ActionMapping &actions, Nebula::Input &input, Nebula::Action action,
                         const char *label)
    {
      ImGui::Text("%s: %s%s", label, actions.isActionDown(action, input) ? "Down " : "",
                  actions.wasActionPressed(action, input) ? "Pressed" : "");
    }
  } // namespace

  Nebula::Entity DebugPanel::findCombatDirectorEntity(Nebula::Scene &scene) const
  {
    for (Nebula::Entity entity : scene.getAllEntities())
    {
      if (!scene.hasComponent<Nebula::ScriptComponent>(entity))
      {
        continue;
      }
      if (scene.getComponent<Nebula::ScriptComponent>(entity).scriptName == "CombatDirector")
      {
        return entity;
      }
    }
    return {};
  }

  void DebugPanel::drawInputOverlay(Nebula::Input &input, Nebula::ActionMapping &actions)
  {
    if (ImGui::CollapsingHeader("Input State", ImGuiTreeNodeFlags_DefaultOpen))
    {
      ImGui::Text("Mouse delta: %.2f, %.2f", input.mouseDeltaX(), input.mouseDeltaY());
      ImGui::Separator();
      ImGui::TextUnformatted("Keys");
      drawKeyState(input, Nebula::Tasto::w);
      drawKeyState(input, Nebula::Tasto::a);
      drawKeyState(input, Nebula::Tasto::s);
      drawKeyState(input, Nebula::Tasto::d);
      drawKeyState(input, Nebula::Tasto::space);
      drawKeyState(input, Nebula::Tasto::left_shift);
      ImGui::Separator();
      ImGui::TextUnformatted("Mouse");
      ImGui::Text("Left: %s%s", input.isMouseButtonDown(Nebula::TastoDelMouse::left) ? "Down " : "",
                  input.wasMouseButtonPressed(Nebula::TastoDelMouse::left) ? "Pressed" : "");
      ImGui::Text("Right: %s%s", input.isMouseButtonDown(Nebula::TastoDelMouse::right) ? "Down " : "",
                  input.wasMouseButtonPressed(Nebula::TastoDelMouse::right) ? "Pressed" : "");
      ImGui::Separator();
      ImGui::TextUnformatted("Actions");
      drawActionState(actions, input, Nebula::Action::MoveForward, "MoveForward");
      drawActionState(actions, input, Nebula::Action::MoveBackward, "MoveBackward");
      drawActionState(actions, input, Nebula::Action::MoveLeft, "MoveLeft");
      drawActionState(actions, input, Nebula::Action::MoveRight, "MoveRight");
      drawActionState(actions, input, Nebula::Action::Jump, "Jump");
      drawActionState(actions, input, Nebula::Action::FastFall, "FastFall");
      drawActionState(actions, input, Nebula::Action::LightAttack, "LightAttack");
      drawActionState(actions, input, Nebula::Action::HeavyAttack, "HeavyAttack");

      float moveX = 0.f;
      float moveY = 0.f;
      float lookX = 0.f;
      float lookY = 0.f;
      actions.getAxisValue(Nebula::Axis::MoveX, input, moveX, moveY);
      actions.getAxisValue(Nebula::Axis::MoveY, input, moveX, moveY);
      actions.getAxisValue(Nebula::Axis::LookX, input, lookX, lookY);
      actions.getAxisValue(Nebula::Axis::LookY, input, lookX, lookY);
      ImGui::Separator();
      ImGui::Text("Axes  move(%.2f, %.2f)  look(%.2f, %.2f)", moveX, moveY, lookX, lookY);
    }
  }

  void DebugPanel::drawCombatTuning(Nebula::Scene &scene, Nebula::ScriptFieldRegistry &fieldRegistry,
                                    EditorState &state, bool isPlaying)
  {
    if (!ImGui::CollapsingHeader("Combat Tuning", ImGuiTreeNodeFlags_DefaultOpen))
    {
      return;
    }

    const Nebula::Entity director = findCombatDirectorEntity(scene);
    if (director.id == 0 || !scene.hasComponent<Nebula::ScriptComponent>(director))
    {
      ImGui::TextDisabled("No CombatDirector entity in scene.");
      return;
    }

    if (isPlaying)
    {
      ImGui::TextColored({0.4f, 0.9f, 0.5f, 1.f}, "Live tuning active (CombatDirector syncs each frame)");
    }

    auto &script = scene.getComponent<Nebula::ScriptComponent>(director);
    script.paramsJson =
        m_scriptParams.mergeScriptParamDefaults(script.paramsJson, fieldRegistry, script.scriptName);

    if (const auto *fields = fieldRegistry.getFields(script.scriptName))
    {
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
        default:
          break;
        }
      }
    }
  }

  void DebugPanel::drawDebugPanel(EditorState &state, Nebula::Scene &scene, Nebula::Input &input,
                                  Nebula::ActionMapping &actions, Nebula::ScriptFieldRegistry &fieldRegistry,
                                  bool isPlaying)
  {
    ImGui::Begin("Debug");
    drawInputOverlay(input, actions);
    drawCombatTuning(scene, fieldRegistry, state, isPlaying);
    ImGui::End();
  }

} // namespace Editor
