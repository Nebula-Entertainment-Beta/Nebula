#include "eventBus.h"

namespace Nebula
{
  void EventBus::push(GameEvent e) { m_queue.push_back(std::move(e)); }
  void EventBus::clear() { m_queue.clear(); }
}