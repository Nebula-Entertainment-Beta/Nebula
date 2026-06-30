#pragma once
#include <string>
namespace Nebula
{
  struct PrefabInstanceComponent
  {
    std::string prefabPath;
    std::string overridesJson = "{}";
  };
}