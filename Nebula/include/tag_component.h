/**
 * @file tag_component.h
 * @brief String tag for scene lookup (`findByTag`); naming is game data, not engine constants.
 */
#pragma once

#include <string>

namespace Nebula
{

  struct TagComponent
  {
    std::string tag;
  };

} // namespace Nebula
