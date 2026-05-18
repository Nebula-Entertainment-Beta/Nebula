#pragma once
#include "eventTypes.h"
#include <variant>
#include <vector>

namespace Nebula
{

  using GameEvent = std::variant<
      WindowResizedEvent,
      SceneSavedEvent,
      InteractPressedEvent,
      SaveSceneRequestedEvent>;

  class EventBus
  {
  public:
    void push(GameEvent e);
    const std::vector<GameEvent> &events() const { return m_queue; }
    void clear(); // call end of frame after subscribers read

    template <typename T>
    bool consumeIf(EventBus &bus, void (*handler)(const T &))
    {
      bool hit = false;
      for (const GameEvent &e : bus.events())
        if (std::holds_alternative<T>(e))
        {
          handler(std::get<T>(e));
          hit = true;
        }
      return hit;
    }

  private:
    std::vector<GameEvent> m_queue;
  };

} // namespace Nebula