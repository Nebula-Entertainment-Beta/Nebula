#pragma once
#include "scene.h"

namespace Nebula
{
  class IAssetProvider;
  class Input;
  class ActionMapping;
  class ScriptRegistry;
  struct FrameInput; // forward; full def in frameCommands.h

  class World
  {
  public:
    World(Scene &scene, IAssetProvider &assets, Input &input,
          ActionMapping &actions, ScriptRegistry &scripts, FrameInput &m_frameInput);

    Scene &scene() { return m_scene; }
    const Scene &scene() const { return m_scene; }
    IAssetProvider &assets() { return m_assets; }
    Input &input() { return m_input; }
    ActionMapping &actions() { return m_actions; }
    ScriptRegistry &scripts() { return m_scripts; }

    FrameInput &frameInput() { return m_frameInput; }
    const FrameInput &frameInput() const { return m_frameInput; }

  private:
    Scene &m_scene;
    IAssetProvider &m_assets;
    Input &m_input;

    ActionMapping &m_actions;
    ScriptRegistry &m_scripts;
    FrameInput &m_frameInput; // or embed FrameInput here; see 6.4
  };
}