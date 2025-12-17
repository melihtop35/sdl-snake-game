#include "../../include/Engine/AudioManager.h"
#include <iostream>

// Singleton erişim noktası
AudioManager &AudioManager::GetInstance()
{
    static AudioManager instance;
    return instance;
}

// Yapıcı
AudioManager::AudioManager()
    : m_sfxVolume(MIX_MAX_VOLUME), m_musicVolume(MIX_MAX_VOLUME), m_isMuted(false), m_isInitialized(false), m_sfxEnabled(true), m_currentMusic("")
{
}

// Yıkıcı
AudioManager::~AudioManager()
{
    Cleanup();
}

// SDL_mixer başlatma
bool AudioManager::Initialize()
{
    if (m_isInitialized)
    {
        return true;
    }

    // SDL_mixer'ı başlat
    // 44100 Hz, varsayılan format, 2 kanal (stereo), 2048 byte buffer
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0)
    {
        std::cerr << "SDL_mixer baslatilamadi! Hata: " << Mix_GetError() << std::endl;
        return false;
    }

    // Kaç kanal kullanılacak (aynı anda çalacak ses sayısı)
    Mix_AllocateChannels(16);

    m_isInitialized = true;
    return true;
}

// Tüm kaynakları temizle
void AudioManager::Cleanup()
{
    // Tüm ses efektlerini serbest bırak
    for (auto &pair : m_soundEffects)
    {
        if (pair.second != nullptr)
        {
            Mix_FreeChunk(pair.second);
        }
    }
    m_soundEffects.clear();

    // Tüm müzikleri serbest bırak
    for (auto &pair : m_music)
    {
        if (pair.second != nullptr)
        {
            Mix_FreeMusic(pair.second);
        }
    }
    m_music.clear();

    // SDL_mixer'ı kapat
    if (m_isInitialized)
    {
        Mix_CloseAudio();
        m_isInitialized = false;
    }

    m_currentMusic = "";
}

// Ses efekti yükle
bool AudioManager::LoadSoundEffect(const std::string &name, const std::string &filePath)
{
    if (!m_isInitialized)
    {
        std::cerr << "AudioManager baslatilmamis!" << std::endl;
        return false;
    }

    // Eğer aynı isimde ses varsa önce sil
    auto it = m_soundEffects.find(name);
    if (it != m_soundEffects.end())
    {
        Mix_FreeChunk(it->second);
        m_soundEffects.erase(it);
    }

    Mix_Chunk *chunk = Mix_LoadWAV(filePath.c_str());
    if (chunk == nullptr)
    {
        std::cerr << "Ses efekti yuklenemedi: " << filePath
                  << " Hata: " << Mix_GetError() << std::endl;
        return false;
    }

    m_soundEffects[name] = chunk;
    return true;
}

// Müzik yükle
bool AudioManager::LoadMusic(const std::string &name, const std::string &filePath)
{
    if (!m_isInitialized)
    {
        std::cerr << "AudioManager baslatilmamis!" << std::endl;
        return false;
    }

    // Eğer aynı isimde müzik varsa önce sil
    auto it = m_music.find(name);
    if (it != m_music.end())
    {
        Mix_FreeMusic(it->second);
        m_music.erase(it);
    }

    Mix_Music *music = Mix_LoadMUS(filePath.c_str());
    if (music == nullptr)
    {
        std::cerr << "Muzik yuklenemedi: " << filePath
                  << " Hata: " << Mix_GetError() << std::endl;
        return false;
    }

    m_music[name] = music;
    return true;
}

// Ses efekti çal
void AudioManager::PlaySoundEffect(const std::string &name, int loops)
{
    if (m_isMuted || !m_isInitialized || !m_sfxEnabled)
    {
        return;
    }

    auto it = m_soundEffects.find(name);
    if (it != m_soundEffects.end())
    {
        // -1: ilk uygun kanalı kullan
        Mix_PlayChannel(-1, it->second, loops);
    }
    else
    {
        std::cerr << "Ses efekti bulunamadi: " << name << std::endl;
    }
}

// Müzik çal
void AudioManager::PlayMusic(const std::string &name, int loops)
{
    if (m_isMuted || !m_isInitialized)
    {
        return;
    }

    auto it = m_music.find(name);
    if (it != m_music.end())
    {
        // loops: -1 = sonsuz döngü
        Mix_PlayMusic(it->second, loops);
        m_currentMusic = name;
    }
    else
    {
        std::cerr << "Muzik bulunamadi: " << name << std::endl;
    }
}

// Müziği durdur
void AudioManager::StopMusic()
{
    Mix_HaltMusic();
    m_currentMusic = "";
}

// Müziği duraklat
void AudioManager::PauseMusic()
{
    Mix_PauseMusic();
}

// Müziğe devam et
void AudioManager::ResumeMusic()
{
    Mix_ResumeMusic();
}

// Müzik çalıyor mu?
bool AudioManager::IsMusicPlaying() const
{
    return Mix_PlayingMusic() != 0;
}

// Ses efekti ses seviyesi
void AudioManager::SetSFXVolume(int volume)
{
    m_sfxVolume = (volume < 0) ? 0 : (volume > MIX_MAX_VOLUME) ? MIX_MAX_VOLUME
                                                               : volume;

    // Tüm kanallara uygula
    Mix_Volume(-1, m_sfxVolume);
}

// Müzik ses seviyesi
void AudioManager::SetMusicVolume(int volume)
{
    m_musicVolume = (volume < 0) ? 0 : (volume > MIX_MAX_VOLUME) ? MIX_MAX_VOLUME
                                                                 : volume;

    Mix_VolumeMusic(m_musicVolume);
}

// Sessiz modu aç/kapat
void AudioManager::ToggleMute()
{
    SetMuted(!m_isMuted);
}

// Sessiz modunu ayarla
void AudioManager::SetMuted(bool muted)
{
    m_isMuted = muted;

    if (m_isMuted)
    {
        Mix_Volume(-1, 0);
        Mix_VolumeMusic(0);
    }
    else
    {
        Mix_Volume(-1, m_sfxVolume);
        Mix_VolumeMusic(m_musicVolume);
    }
}

// Ses efekti yüklü mü?
bool AudioManager::IsSoundLoaded(const std::string &name) const
{
    return m_soundEffects.find(name) != m_soundEffects.end();
}

// Müzik yüklü mü?
bool AudioManager::IsMusicLoaded(const std::string &name) const
{
    return m_music.find(name) != m_music.end();
}
