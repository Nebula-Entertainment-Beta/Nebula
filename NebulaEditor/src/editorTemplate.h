#pragma once
#include <cstring>
#include "Nebula.h"

namespace Editor
{

  class EditorTemplate
  {

  public:
    Nebula::Entity createEmpty(Nebula::Scene &scene);
    Nebula::Entity createMeshCube(Nebula::Scene &scene, const char *tag = "Cube");
    Nebula::Entity createEnemyPlaceholder(Nebula::Scene &scene);
    Nebula::Entity createPlatform(Nebula::Scene &scene, Nebula::AssetManager &assets);
    Nebula::Entity createBouncePad(Nebula::Scene &scene, Nebula::AssetManager &assets);
    Nebula::Entity createWindVolume(Nebula::Scene &scene, Nebula::AssetManager &assets);
    Nebula::Entity createStaticMesh(Nebula::Scene &scene, Nebula::AssetManager &assets,
                                    const Nebula::Vec3 &position, std::string_view meshPath,
                                    std::string_view materialPath, const char *tag = "Prop");
  };
}