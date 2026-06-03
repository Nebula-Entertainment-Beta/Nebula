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
  };
}