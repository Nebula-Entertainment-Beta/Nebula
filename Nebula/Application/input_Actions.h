/**
 * @file input_Actions.h
 * @brief Semantic **actions** and **axes** built on top of raw `Input` (keys, mouse delta, scroll).
 *
 * **Owns:** `ActionMapping`, enums `Action` / `Axis`, `AxisBinding`, and `CameraSensitivity`.
 *
 * **Why:** Gameplay asks for “move forward” or “look X”, not “is GLFW_KEY_W down”, so bindings,
 * sensitivity, and future rebinding stay centralized. `updateMappings` reads `Input` each frame and
 * fills cached axis values used by `getAxisValue` and action queries.
 */
#pragma once
#include "input.h"
#include <unordered_map>
#include <vector>
#include <glm/glm.hpp>
#include <unordered_set>

namespace Nebula {

  enum class Action {
    MoveForward,
    MoveBackward,
    MoveLeft,
    MoveRight,
    Jump,
    Sprint,
    Hit,
    Interact
  };

  enum class Axis {
    MoveX,
    MoveY,
    LookX,
    LookY,
    Scroll
  };

  enum class AxisSourceType{
    KeyPair,
    MouseDeltaX,
    MouseDeltaY,
    MouseScrollY
  };

  struct AxisBinding {
    AxisSourceType source = AxisSourceType::KeyPair;
    int positiveKey = -1;
    int negativeKey = -1;
    float scale = 1.0f;
  };

  struct CameraSensitivity{
    float lookSensitivityX = 1.0f;
    float lookSensitivityY = 1.0f;
    float zoomSensitivity = 1.0f;
    bool invertLookY = false;
  };


  class ActionMapping{
    public:
      ActionMapping();
      void bindDefaultActions();
      void setupDefaultBindings();
      void bindAction(Action action, int key) {
        
        m_actionBindings[action].push_back(key);
      }

      void bindAxis(Axis axis, const AxisBinding& binding) {
          m_axisBindings[axis].push_back(binding);
      }
      void updateMappings(const Input& input);
      void getAxisValue(Axis axis, const Input& input, float& outX, float& outY) const;
      bool isActionDown(Action action, const Input& input) const;
      bool wasActionPressed(Action action, const Input& input) const;
      bool wasActionReleased(Action action, const Input& input) const;
      void clearBindings();
      void clearActionBindings(Action action);
      void clearAxisBindings(Axis axis);
      void addAxisBinding(Axis axis, const AxisBinding& binding);
      void addActionBinding(Action action, int key);
      void warnOnce(const std::string& message);

      void getCameraSensitivity(float& outLookSensitivityX, float& outLookSensitivityY, float& outZoomSensitivity, bool& outInvertLookY) const{
        outLookSensitivityX = m_cameraSensitivity.lookSensitivityX;
        outLookSensitivityY = m_cameraSensitivity.lookSensitivityY;
        outZoomSensitivity = m_cameraSensitivity.zoomSensitivity;
        outInvertLookY = m_cameraSensitivity.invertLookY;
      }

      void setCameraSensitivity(float lookSensitivityX, float lookSensitivityY, float zoomSensitivity, bool invertLookY){
        m_cameraSensitivity.lookSensitivityX = lookSensitivityX;
        m_cameraSensitivity.lookSensitivityY = lookSensitivityY;
        m_cameraSensitivity.zoomSensitivity = zoomSensitivity;
        m_cameraSensitivity.invertLookY = invertLookY;
      }

      

      

    private:
      std::unordered_map<Action, std::vector<int>> m_actionBindings;
      std::unordered_map<Axis, std::vector<AxisBinding>> m_axisBindings;
      std::unordered_map<Action, bool> m_actionStates;
      std::unordered_map<Axis, glm::vec3> m_axisValues;
      CameraSensitivity m_cameraSensitivity;
      std::unordered_set<std::string> m_warnedMessages;

      

  };
}