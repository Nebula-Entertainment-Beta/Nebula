#pragma once

namespace Nebula
{
  class World;
  class EventBus;

  struct FrameInput
  {
    float moveX = 0.f, moveY = 0.f;
    float lookX = 0.f, lookY = 0.f;
    float zoomX = 0.f, zoomY = 0.f;

    void clear()
    {
      moveX = moveY = lookX = lookY = zoomX = zoomY = 0.f;
    }
  };

  void buildFrameInput(World &world, EventBus &bus);
}