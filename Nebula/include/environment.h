#pragma once

#include "component.h"
#include "math_types.h"

namespace Nebula
{
  class Scene;

  EnvironmentComponent findEnvironmentOrDefault(const Scene &scene);
  Vec4 environmentClearColor(const EnvironmentComponent &env);
}
