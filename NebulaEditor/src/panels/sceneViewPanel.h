#pragma once

#include <functional>
#include <string_view>

#include "editorState.h"
#include "Nebula.h"
#include "render/sceneViewFrameBuffer.h"

namespace Nebula
{
  class Input;
}

namespace Editor
{

  class SceneViewPanel
  {
  public:
    using SpawnMeshFn = std::function<void(std::string_view meshPath, std::string_view materialPath,
                                           const Nebula::Vec3 &position)>;
    using SpawnPrefabFn = std::function<void(std::string_view prefabPath, const Nebula::Vec3 &position)>;

    void drawSceneViewPanel(EditorState &state, SceneViewFrameBuffer &framebuffer, Nebula::Scene &scene,
                            Nebula::AssetManager &assets, Nebula::Renderer &renderer, Nebula::Window &window,
                            Nebula::Input &input, bool isPlaying, const SpawnMeshFn &spawnMeshAt,
                            const SpawnPrefabFn &spawnPrefabAt);
  };

} // namespace Editor
