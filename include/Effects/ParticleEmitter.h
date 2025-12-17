#pragma once
#include "Particle.h"
#include <vector>
#include <SDL2/SDL.h>

/**
 * @brief Yayılma şekli enum'u
 */
enum class EmissionShape
{
    POINT,     // Tek noktadan yayılım
    CIRCLE,    // Daire şeklinde yayılım
    RECTANGLE, // Dikdörtgen alanda yayılım
    CONE       // Koni şeklinde yayılım
};

/**
 * @brief Blend modu enum'u
 */
enum class ParticleBlendMode
{
    NORMAL,  // Normal karışım
    ADDITIVE // Additif (parlak efektler için)
};

/**
 * @brief Parçacık yayıcı sınıfı
 *
 * Parçacıkları oluşturur, günceller ve render eder.
 * Object pooling kullanarak performanslı çalışır.
 */
class ParticleEmitter
{
public:
    ParticleEmitter(int maxParticles = 100);
    ~ParticleEmitter();

    // Ana fonksiyonlar
    void Update(float deltaTime);
    void Render(SDL_Renderer *renderer);

    // Parçacık üretimi
    void Emit(int count = 1); // Belirli sayıda parçacık üret
    void Burst(int count);    // Anlık patlama efekti
    void Start();             // Sürekli yayılımı başlat
    void Stop();              // Sürekli yayılımı durdur

    // Konum
    void SetPosition(float x, float y);
    void GetPosition(float &x, float &y) const;

    // Emission ayarları
    void SetEmissionRate(float rate) { m_emissionRate = rate; }
    void SetEmissionShape(EmissionShape shape) { m_emissionShape = shape; }
    void SetEmissionAngle(float angle) { m_emissionAngle = angle; }
    void SetSpreadAngle(float spread) { m_spreadAngle = spread; }
    void SetEmissionRadius(float radius) { m_emissionRadius = radius; }

    // Parçacık özellikleri
    void SetLifetime(float min, float max)
    {
        m_minLifetime = min;
        m_maxLifetime = max;
    }
    void SetSpeed(float min, float max)
    {
        m_minSpeed = min;
        m_maxSpeed = max;
    }
    void SetSize(float min, float max)
    {
        m_minSize = min;
        m_maxSize = max;
    }
    void SetRotationSpeed(float min, float max)
    {
        m_minRotationSpeed = min;
        m_maxRotationSpeed = max;
    }

    // Renk ayarları
    void SetStartColor(Uint8 r, Uint8 g, Uint8 b, Uint8 a = 255);
    void SetEndColor(Uint8 r, Uint8 g, Uint8 b, Uint8 a = 0);

    // Fizik
    void SetGravity(float x, float y)
    {
        m_gravityX = x;
        m_gravityY = y;
    }

    // Görsel
    void SetBlendMode(ParticleBlendMode mode) { m_blendMode = mode; }
    void SetTexture(SDL_Texture *texture) { m_texture = texture; }

    // Durum sorgulama
    bool IsActive() const { return m_isActive; }
    int GetActiveParticleCount() const;
    bool HasAliveParticles() const;

    // Temizlik
    void Clear();

private:
    // Yardımcı fonksiyonlar
    Particle *FindDeadParticle();
    void SpawnParticle();
    void UpdateParticle(Particle &particle, float deltaTime);
    void RenderParticle(SDL_Renderer *renderer, const Particle &particle);
    float RandomFloat(float min, float max);
    Uint8 Lerp(Uint8 a, Uint8 b, float t);

private:
    // Parçacık havuzu
    std::vector<Particle> m_particles;
    int m_maxParticles;

    // Konum
    float m_posX, m_posY;

    // Emission ayarları
    float m_emissionRate;        // Saniyede üretilecek parçacık
    float m_emissionAccumulator; // Birikmiş emission zamanı
    EmissionShape m_emissionShape;
    float m_emissionAngle;  // Derece cinsinden
    float m_spreadAngle;    // Yayılma açısı
    float m_emissionRadius; // Daire/dikdörtgen için

    // Parçacık özellikleri (min/max)
    float m_minLifetime, m_maxLifetime;
    float m_minSpeed, m_maxSpeed;
    float m_minSize, m_maxSize;
    float m_minRotationSpeed, m_maxRotationSpeed;

    // Renkler
    Uint8 m_startR, m_startG, m_startB, m_startA;
    Uint8 m_endR, m_endG, m_endB, m_endA;

    // Fizik
    float m_gravityX, m_gravityY;

    // Görsel
    ParticleBlendMode m_blendMode;
    SDL_Texture *m_texture;

    // Durum
    bool m_isActive;
};
