/**
 * @file input_Actions.cpp
 * @brief Default WASD + mouse-look bindings and axis aggregation for `ActionMapping`.
 */
#include "input_Actions.h"
#include <GLFW/glfw3.h>
#include <algorithm>
#include <iostream>

namespace Nebula {
  ActionMapping::ActionMapping(){
    clearBindings();
    bindDefaultActions();
    setupDefaultBindings();
    
    
  }

  void ActionMapping::bindDefaultActions(){
    m_actionBindings[Action::MoveForward] = {};
    m_actionBindings[Action::MoveBackward] = {};
    m_actionBindings[Action::MoveLeft] = {};
    m_actionBindings[Action::MoveRight] = {};
    m_actionBindings[Action::Jump] = {};
    m_actionBindings[Action::Sprint] = {};


    m_axisBindings[Axis::MoveX] = {};
    m_axisBindings[Axis::MoveY] = {};
    m_axisBindings[Axis::LookX] = {};
    m_axisBindings[Axis::LookY] = {};
    m_axisBindings[Axis::Scroll] = {};

  }

  void ActionMapping::setupDefaultBindings(){
    //error if no binding present for an expected axis
    

    bindAction(Action::MoveForward, GLFW_KEY_W);
    bindAction(Action::MoveBackward, GLFW_KEY_S);
    bindAction(Action::MoveLeft, GLFW_KEY_A);
    bindAction(Action::MoveRight, GLFW_KEY_D);
    bindAction(Action::Interact, GLFW_KEY_TAB);
    bindAxis(Axis::MoveX, AxisBinding{AxisSourceType::KeyPair, GLFW_KEY_D, GLFW_KEY_A, 1.0f});
    bindAxis(Axis::MoveY, AxisBinding{AxisSourceType::KeyPair, GLFW_KEY_W, GLFW_KEY_S, 1.0f});
    bindAxis(Axis::LookX, AxisBinding{AxisSourceType::MouseDeltaX, -1, -1, 1.0f});
    bindAxis(Axis::LookY, AxisBinding{AxisSourceType::MouseDeltaY, -1, -1, 1.0f});
    bindAxis(Axis::Scroll, AxisBinding{AxisSourceType::MouseScrollY, -1, -1, 1.0f});
    

    if (m_axisBindings[Axis::MoveX].empty()) {
        std::cerr << "Error: No binding present for expected axis MoveX." << std::endl;
    }
    if (m_axisBindings[Axis::MoveY].empty()) {
        std::cerr << "Error: No binding present for expected axis MoveY." << std::endl;
    }
    if (m_axisBindings[Axis::LookX].empty()) {
        std::cerr << "Error: No binding present for expected axis LookX." << std::endl;
    }
    if (m_axisBindings[Axis::LookY].empty()) {
        std::cerr << "Error: No binding present for expected axis LookY." << std::endl;
    }
    if (m_axisBindings[Axis::Scroll].empty()) {
        std::cerr << "Error: No binding present for expected axis Scroll." << std::endl;
    }
    

  }

  void ActionMapping::updateMappings(const Input& input) {
        //same key as both postive should cancel out and add obvious debug statements
        if(m_axisBindings[Axis::MoveX].size() > 0){
            for(const AxisBinding& binding : m_axisBindings[Axis::MoveX]){
                if(binding.source == AxisSourceType::KeyPair && binding.positiveKey == binding.negativeKey){
                    warnOnce("Warning: MoveX axis has a key pair binding with the same key for both positive and negative.");
                    }
            }
        }

        if(m_axisBindings[Axis::MoveY].size() > 0){
            for(const AxisBinding& binding : m_axisBindings[Axis::MoveY]){
                if(binding.source == AxisSourceType::KeyPair && binding.positiveKey == binding.negativeKey){
                    warnOnce("Warning: MoveY axis has a key pair binding with the same key for both positive and negative.");
                }
            }
        }

        if(m_axisBindings[Axis::LookX].size() > 0){
            for(const AxisBinding& binding : m_axisBindings[Axis::LookX]){
                if(binding.source == AxisSourceType::KeyPair && binding.positiveKey == binding.negativeKey){
                    warnOnce("Warning: LookX axis has a key pair binding with the same key for both positive and negative.");
                }
            }
        }

        if(m_axisBindings[Axis::LookY].size() > 0){
            for(const AxisBinding& binding : m_axisBindings[Axis::LookY]){
                if(binding.source == AxisSourceType::KeyPair && binding.positiveKey == binding.negativeKey){
                    warnOnce("Warning: LookY axis has a key pair binding with the same key for both positive and negative.");
                }
            }
        }

        if(m_axisBindings[Axis::Scroll].size() > 0){
            for(const AxisBinding& binding : m_axisBindings[Axis::Scroll]){
                if(binding.source == AxisSourceType::KeyPair && binding.positiveKey == binding.negativeKey){
                    warnOnce("Warning: Scroll axis has a key pair binding with the same key for both positive and negative.");
                }
            }
        }

        for (auto& [action, keys] : m_actionBindings) {
            bool isDown = false;
            for (int key : keys) {
                if (input.isKeyDown(key)) {
                    isDown = true;
                    break;
                }
            }
            m_actionStates[action] = isDown;
        }


        for (auto& [axis, bindings] : m_axisBindings) {
              std::array<float, 3> value{{0.0f, 0.0f, 0.0f}};
              for (const AxisBinding& binding : bindings) {
                  switch (binding.source) {
                      case AxisSourceType::KeyPair: {
                          float keyAxis = 0.0f;
                          if (binding.positiveKey >= 0 && input.isKeyDown(binding.positiveKey)) {
                              keyAxis += 1.0f;
                          }
                          if (binding.negativeKey >= 0 && input.isKeyDown(binding.negativeKey)) {
                              keyAxis -= 1.0f;
                          }
                          if (axis == Axis::MoveX) {
                              value[0] += keyAxis * binding.scale;
                          } else if (axis == Axis::MoveY) {
                              value[1] += keyAxis * binding.scale;
                          }
                          break;
                      }
                      case AxisSourceType::MouseDeltaX:
                          value[0] += input.mouseDeltaX() * binding.scale * m_cameraSensitivity.lookSensitivityX;
                          break;
                      case AxisSourceType::MouseDeltaY:
                          value[1] += input.mouseDeltaY() * binding.scale * m_cameraSensitivity.lookSensitivityY * (m_cameraSensitivity.invertLookY ? -1.0f : 1.0f);
                          break;
                      case AxisSourceType::MouseScrollY:
                          value[1] += input.mouseScrollDeltaY() * binding.scale * m_cameraSensitivity.zoomSensitivity;
                          break;
                  }
              }
              m_axisValues[axis] = value;
          }
         


          //apply scale or clamp wherever necessary
          for (auto& [axis, value] : m_axisValues) {
              if (axis == Axis::MoveX || axis == Axis::MoveY) {
                  value[0] = std::clamp(value[0], -1.0f, 1.0f);
                  value[1] = std::clamp(value[1], -1.0f, 1.0f);
              }
          }


  }

  bool ActionMapping::isActionDown(Action action, const Input& input) const {
      auto it = m_actionBindings.find(action);
      if (it != m_actionBindings.end()) {
          for (int key : it->second) {
              if (input.isKeyDown(key)) {
                  return true;
              }
          }
      }
      return false;
  }

  bool ActionMapping::wasActionPressed(Action action, const Input& input) const {
      auto it = m_actionBindings.find(action);
      if (it != m_actionBindings.end()) {
          for (int key : it->second) {
              if (input.wasKeyPressed(key)) {
                  return true;
              }
          }
      }
      return false;
  }

  bool ActionMapping::wasActionReleased(Action action, const Input& input) const {
      auto it = m_actionBindings.find(action);
      if (it != m_actionBindings.end()) {
          for (int key : it->second) {
              if (input.wasKeyReleased(key)) {
                  return true;
              }
          }
      }
      return false;
  }

  void ActionMapping::getAxisValue(Axis axis, const Input& input, float& outX, float& outY) const {
      (void)input;
      auto it = m_axisValues.find(axis);
      if (it != m_axisValues.end()) {
          outX += it->second[0];
          outY += it->second[1];
      }
  }
  
  void ActionMapping::clearBindings() {
      m_actionBindings.clear();
      m_axisBindings.clear();
  }

  void ActionMapping::clearActionBindings(Action action) {
      m_actionBindings[action].clear();
  }

  void ActionMapping::addActionBinding(Action action, int key) {
      m_actionBindings[action].push_back(key);
  }

  void ActionMapping::clearAxisBindings(Axis axis) {
      m_axisBindings[axis].clear();
  }

  void ActionMapping::addAxisBinding(Axis axis, const AxisBinding& binding) {
      m_axisBindings[axis].push_back(binding);
  }

  void ActionMapping::warnOnce(const std::string&message) {
    //insert message into set if it doesnt exit 
    auto [it, inserted] = m_warnedMessages.insert(message);
    if(inserted){
        std::cerr << message << std::endl;
    }
    

  }
  

}


