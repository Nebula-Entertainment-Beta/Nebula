#pragma once
#include <cstdint>
#include <string>

namespace Nebula
{

  enum class EventType : uint8_t
  {
    WindowResized,
    SceneSaved,
    InteractPressed,
    SaveSceneRequested,
  };

  struct WindowResizedEvent
  {
    int width;
    int height;
  };
  struct SceneSavedEvent
  {
    std::string path;
  };
  struct InteractPressedEvent
  {
  };
  struct SaveSceneRequestedEvent
  {
  };

} // namespace Nebula