#pragma once

#include <memory>
#include <string_view>

namespace Nebula
{

  class IAudioService
  {
  public:
    virtual ~IAudioService() = default;
    virtual void playOneShot(std::string_view logicalPath, float volume = 1.f) = 0;
    virtual void update() {}
  };

  /** FMOD Core backend when FMOD_ROOT / find_package succeeds; otherwise a logging stub. */
  std::unique_ptr<IAudioService> createAudioService();

}
