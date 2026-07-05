#ifndef AUDIO_ENGINE_HPP
#define AUDIO_ENGINE_HPP

#include "miniaudio.h"

// SingleTon Audio provider which can be used in any file
class AudioEngine
{
public:
    static AudioEngine& Instance();

    AudioEngine(const AudioEngine&) = delete;
    AudioEngine& operator=(const AudioEngine&) = delete;

    void PlayMusic(const std::string& path = "assets/sounds/main_game_theme.ogg");
    void StopMusic();

    void PlayShortSound(const std::string& path);

    void changeVolume(float volume = 50.0f);
private:
    ma_engine m_engine;
    ma_sound m_musicSound;
    bool m_has_music;
    bool m_initialized;

    AudioEngine();
    ~AudioEngine();
};

#endif // AUDIO_ENGINE_HPP