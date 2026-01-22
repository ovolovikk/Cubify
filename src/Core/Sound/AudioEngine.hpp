#ifndef AUDIO_ENGINE_HPP
#define AUDIO_ENGINE_HPP

#include "miniaudio.h"

#include <string>

class AudioEngine
{
public:
    static AudioEngine& Instance();

    AudioEngine(const AudioEngine&) = delete;
    AudioEngine& operator=(const AudioEngine&) = delete;

    void PlayMusic(const std::string& path = "assets/sounds/interstellar.mp3");

    void PlayShortSound(const std::string& path);

    void changeVolume(float volume = 50.0f);
private:
    ma_engine m_engine;
    bool m_initialized;

    AudioEngine();
    ~AudioEngine();
};

#endif // AUDIO_ENGINE_HPP