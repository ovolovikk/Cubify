#include "Core/Sound/AudioEngine.hpp"

#include "Core/Logging/Log.hpp"

#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"

AudioEngine& AudioEngine::Instance()
{
    static AudioEngine s_instance;
    return s_instance;
}

AudioEngine::AudioEngine()
{
    LOGI("[AudioEngine] Initializing");
    ma_result result;
    result = ma_engine_init(NULL, &m_engine);
    if (result != MA_SUCCESS) {
        LOGE("[AudioEngine] Failed to initialize");
        m_initialized = false;
    } else {
        m_initialized = true;
    }
}

AudioEngine::~AudioEngine()
{
    if (m_initialized) 
    {
        LOGI("[AudioEngine] was destroyed");
        ma_engine_uninit(&m_engine);
    }
}

void AudioEngine::PlayMusic(const std::string& path)
{
    if (!m_initialized)
    {
        LOGE("[AudioEngine] Tried to play music but uninitialized");
        return;
    }
    ma_engine_play_sound(&m_engine, path.c_str(), NULL);
}

void AudioEngine::PlayShortSound(const std::string& path)
{
    if(!m_initialized)
    {
        LOGE("[AudioEngine] Tried to play sound but uninitialized");
        return;
    }

    ma_engine_play_sound(&m_engine, path.c_str(), NULL);
}

void AudioEngine::changeVolume(float volume)
{
    volume = static_cast<float>(volume) / 100.0f;
    ma_engine_set_volume(&m_engine, volume);
}