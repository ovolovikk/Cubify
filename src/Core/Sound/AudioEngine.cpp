#include "Core/Sound/AudioEngine.hpp"
#include "Core/Logging/Log.hpp"

#define STB_VORBIS_HEADER_ONLY
#include "stb_vorbis.c" 

#define MINIAUDIO_IMPLEMENTATION
#define MA_ENABLE_VORBIS
#include "miniaudio.h"

#undef STB_VORBIS_HEADER_ONLY
#include "stb_vorbis.c"

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
        changeVolume();
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
        LOGE("[AudioEngine] Tried to play the music but uninitialized");
        return;
    }

    StopMusic();
    
    ma_result result = ma_sound_init_from_file(&m_engine,
                                    path.c_str(),
                                    MA_SOUND_FLAG_STREAM,
                                    nullptr,
                                    nullptr,
                                    &m_musicSound);
    
    if (result != MA_SUCCESS)
    {
        LOGE("[AudioEngine] Failed to play the music %s", ma_result_description(result));
        return;
    }

    ma_sound_set_looping(&m_musicSound, MA_TRUE);
    ma_sound_start(&m_musicSound);

    m_has_music = true;
}

void AudioEngine::StopMusic()
{
    if(m_has_music)
    {
        LOGI("[AudioEngine][StopMusic] Stopping music");
        ma_sound_stop(&m_musicSound);
        ma_sound_uninit(&m_musicSound);
        m_has_music = false;
    }
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