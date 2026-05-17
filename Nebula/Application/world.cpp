#include "world.h"
#include "frameCommands.h"
#include "assetProvider.h"
#include "input.h"
#include "input_Actions.h"
#include "script_Registry.h"

namespace Nebula
{
  World::World(Scene &scene, IAssetProvider &assets, Input &input, ActionMapping &actions, ScriptRegistry &scripts, FrameInput &frame) : m_scene(scene), m_assets(assets), m_input(input), m_actions(actions), m_scripts(scripts), m_frameInput(frame) {}
}