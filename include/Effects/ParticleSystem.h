#pragma once
#include "ParticleEmitter.h"
#include <vector>
#include <memory>
#include <string>
#include <unordered_map>

/**
 * @brief Parçacık sistemi yönetici sınıfı (Singleton)
 *
 * Tüm parçacık emitter'larını yönetir.
 * Hazır efekt preset'leri sunar.
 */
class ParticleSystem
{
public:
    // Singleton erişimi
    static ParticleSystem &GetInstance();

    // Silinen fonksiyonlar (Singleton pattern)
    ParticleSystem(const ParticleSystem &) = delete;
    ParticleSystem &operator=(const ParticleSystem &) = delete;

    // Yaşam döngüsü
    void Initialize();
    void Update(float deltaTime);
    void Render(SDL_Renderer *renderer);
    void Cleanup();

    // Emitter yönetimi
    ParticleEmitter *CreateEmitter(const std::string &name, int maxParticles = 100);
    ParticleEmitter *GetEmitter(const std::string &name);
    void RemoveEmitter(const std::string &name);
    void ClearAllEmitters();

    // Hazır efekt preset'leri - Kolay kullanım için
    void PlayExplosion(float x, float y, int particleCount = 30);
    void PlaySparkle(float x, float y, int particleCount = 15);
    void PlaySmoke(float x, float y);
    void PlayFirework(float x, float y);
    void PlayCollect(float x, float y); // Yem toplama efekti
    void PlayDeath(float x, float y);   // Ölüm efekti

    // Food efektleri - Yem türüne göre parçacık efekti
    void PlayFoodCollect(float x, float y, int foodType);

    // Bomba hasar efekti - Yılanın shrink olan parçası için
    void PlayBombDamage(float x, float y);

    // Yılan ölüm efekti - Sıralı patlama için segment listesi alır
    // delay: Her segment arasındaki gecikme (saniye)
    void PlaySnakeDeathSequence(const std::vector<std::pair<float, float>> &segments, float delay = 0.05f);

    // Sıralı efekt güncelleme (internal)
    void UpdateSequencedEffects(float deltaTime);

    // One-shot efektler (otomatik temizlenir)
    void SpawnOneShot(float x, float y,
                      int particleCount,
                      float minSpeed, float maxSpeed,
                      float minLifetime, float maxLifetime,
                      Uint8 startR, Uint8 startG, Uint8 startB,
                      Uint8 endR, Uint8 endG, Uint8 endB,
                      float gravity = 0,
                      float minSize = 3, float maxSize = 8);

    // Durum
    int GetTotalParticleCount() const;
    int GetActiveEmitterCount() const;

private:
    ParticleSystem();
    ~ParticleSystem();

    // Temizlik - ölü one-shot emitter'ları sil
    void CleanupDeadEmitters();

private:
    // İsimli emitter'lar (sürekli kullanım için)
    std::unordered_map<std::string, std::unique_ptr<ParticleEmitter>> m_namedEmitters;

    // One-shot emitter'lar (otomatik temizlenir)
    std::vector<std::unique_ptr<ParticleEmitter>> m_oneShotEmitters;

    // Temizlik sayacı
    float m_cleanupTimer;
    static constexpr float CLEANUP_INTERVAL = 2.0f; // Her 2 saniyede temizle

    // Sıralı efekt sistemi (yılan ölümü için)
    struct SequencedEffect
    {
        std::vector<std::pair<float, float>> positions; // Patlama pozisyonları
        float delay;                                    // Her patlama arasındaki gecikme
        float timer;                                    // Geçen süre
        int currentIndex;                               // Şu anki patlama indeksi
        bool isActive;
    };
    std::vector<SequencedEffect> m_sequencedEffects;
};
