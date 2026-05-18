/**
 * @file entity.h
 * @brief Entity handle: stable id plus generation to detect use-after-destroy.
 */
#pragma once
#include <cstdint>

namespace Nebula
{

  using EntityID = uint32_t;

  struct Entity
  {
    EntityID id = 0;
    uint32_t generation = 0;

    friend bool operator==(const Entity &a, const Entity &b)
    {
      return a.id == b.id && a.generation == b.generation;
    }

    friend bool operator!=(const Entity &a, const Entity &b) { return !(a == b); }
  };

} // namespace Nebula
