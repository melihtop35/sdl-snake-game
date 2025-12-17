#pragma once

#include <SDL2/SDL_mixer.h>
#include <string>
#include <unordered_map>

class AudioManager
{
private:
    // Ses efektleri (kısa sesler: yeme, hareket, vb.)
    std::unordered_map<std::string, Mix_Chunk *> m_soundEffects;

    // Müzikler (uzun arka plan müzikleri)
    std::unordered_map<std::string, Mix_Music *> m_music;

    // Ses seviyeleri (0-128 arası, SDL_mixer standardı)
    int m_sfxVolume;
    int m_musicVolume;

    // Ses durumu
    bool m_isMuted;
    bool m_isInitialized;
    bool m_sfxEnabled; // Ses efektleri aktif mi

    // Şu an çalan müzik
    std::string m_currentMusic;

    AudioManager();
    AudioManager(const AudioManager &) = delete;
    AudioManager &operator=(const AudioManager &) = delete;

public:
    static AudioManager &GetInstance();
    ~AudioManager();

    // Başlatma ve temizleme
    bool Initialize();
    void Cleanup();

    // Yükleme fonksiyonları
    bool LoadSoundEffect(const std::string &name, const std::string &filePath);
    bool LoadMusic(const std::string &name, const std::string &filePath);

    // Çalma fonksiyonları
    void PlaySoundEffect(const std::string &name, int loops = 0);
    void PlayMusic(const std::string &name, int loops = -1); // -1 = sonsuz döngü

    // Müzik kontrolü
    void StopMusic();
    void PauseMusic();
    void ResumeMusic();
    bool IsMusicPlaying() const;

    // Ses seviyesi kontrolü
    void SetSFXVolume(int volume);   // 0-128
    void SetMusicVolume(int volume); // 0-128
    int GetSFXVolume() const { return m_sfxVolume; }
    int GetMusicVolume() const { return m_musicVolume; }

    // Ses efektleri aç/kapa
    void SetSFXEnabled(bool enabled) { m_sfxEnabled = enabled; }
    bool IsSFXEnabled() const { return m_sfxEnabled; }

    // Sessiz mod
    void ToggleMute();
    void SetMuted(bool muted);
    bool IsMuted() const { return m_isMuted; }

    // Yardımcı
    bool IsSoundLoaded(const std::string &name) const;
    bool IsMusicLoaded(const std::string &name) const;
};
