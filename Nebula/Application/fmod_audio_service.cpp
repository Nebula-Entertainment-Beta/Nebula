#include "audioService.h"
#include "logSink.h"

#include <iostream>
#include <string>

#if defined(NEBULA_USE_FMOD)
#include "fmod.hpp"
#include "fmod_errors.h"
#endif

namespace Nebula
{
  namespace
  {
    class StubAudioService final : public IAudioService
    {
    public:
      void playOneShot(std::string_view logicalPath, float) override
      {
        std::cout << "[Audio] (stub) play " << logicalPath << '\n';
      }
    };

#if defined(NEBULA_USE_FMOD)
    class FmodAudioService final : public IAudioService
    {
    public:
      FmodAudioService()
      {
        if (FMOD::System_Create(&m_system) != FMOD_OK)
        {
          return;
        }
        if (m_system->init(32, FMOD_INIT_NORMAL, nullptr) != FMOD_OK)
        {
          m_system->release();
          m_system = nullptr;
        }
      }

      ~FmodAudioService() override
      {
        if (m_system != nullptr)
        {
          m_system->close();
          m_system->release();
        }
      }

      void playOneShot(std::string_view logicalPath, float volume) override
      {
        if (m_system == nullptr)
        {
          return;
        }
        FMOD::Sound *sound = nullptr;
        const std::string path(logicalPath);
        if (m_system->createSound(path.c_str(), FMOD_DEFAULT | FMOD_CREATESTREAM, nullptr, &sound) !=
            FMOD_OK)
        {
          std::cerr << "[Audio] FMOD failed to load " << path << '\n';
          return;
        }
        FMOD::Channel *channel = nullptr;
        m_system->playSound(sound, nullptr, false, &channel);
        if (channel != nullptr)
        {
          channel->setVolume(volume);
        }
        sound->setMode(FMOD_LOOP_OFF);
        // Fire-and-forget: release after play completes via non-blocking update.
        sound->release();
      }

      void update() override
      {
        if (m_system != nullptr)
        {
          m_system->update();
        }
      }

    private:
      FMOD::System *m_system = nullptr;
    };
#endif
  }

  std::unique_ptr<IAudioService> createAudioService()
  {
#if defined(NEBULA_USE_FMOD)
    auto svc = std::make_unique<FmodAudioService>();
    return svc;
#else
    return std::make_unique<StubAudioService>();
#endif
  }

}
