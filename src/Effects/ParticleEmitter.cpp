#include "../../include/Effects/ParticleEmitter.h"
#include <cmath>
#include <cstdlib>
#include <ctime>

// Pi sabiti
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

ParticleEmitter::ParticleEmitter(int maxParticles)
    : m_maxParticles(maxParticles), m_posX(0), m_posY(0), m_emissionRate(10.0f), m_emissionAccumulator(0), m_emissionShape(EmissionShape::POINT), m_emissionAngle(-90.0f) // Yukarı doğru (derece)
      ,
      m_spreadAngle(360.0f) // Tam daire
      ,
      m_emissionRadius(10.0f), m_minLifetime(0.5f), m_maxLifetime(1.5f), m_minSpeed(50.0f), m_maxSpeed(150.0f), m_minSize(3.0f), m_maxSize(8.0f), m_minRotationSpeed(-180.0f), m_maxRotationSpeed(180.0f), m_startR(255), m_startG(255), m_startB(255), m_startA(255), m_endR(255), m_endG(255), m_endB(255), m_endA(0), m_gravityX(0), m_gravityY(0), m_blendMode(ParticleBlendMode::NORMAL), m_texture(nullptr), m_isActive(false)
{
    // Rastgele sayı üreteci için seed
    static bool seeded = false;
    if (!seeded)
    {
        srand(static_cast<unsigned int>(time(nullptr)));
        seeded = true;
    }

    // Parçacık havuzunu oluştur
    m_particles.resize(maxParticles);
}

ParticleEmitter::~ParticleEmitter()
{
    Clear();
}

void ParticleEmitter::Update(float deltaTime)
{
    // Sürekli yayılım aktifse yeni parçacık üret
    if (m_isActive && m_emissionRate > 0)
    {
        m_emissionAccumulator += deltaTime;
        float emissionInterval = 1.0f / m_emissionRate;

        while (m_emissionAccumulator >= emissionInterval)
        {
            SpawnParticle();
            m_emissionAccumulator -= emissionInterval;
        }
    }

    // Tüm parçacıkları güncelle
    for (auto &particle : m_particles)
    {
        if (particle.isAlive)
        {
            UpdateParticle(particle, deltaTime);
        }
    }
}

void ParticleEmitter::Render(SDL_Renderer *renderer)
{
    // Blend modunu ayarla
    if (m_blendMode == ParticleBlendMode::ADDITIVE)
    {
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_ADD);
    }
    else
    {
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    }

    // Tüm canlı parçacıkları render et
    for (const auto &particle : m_particles)
    {
        if (particle.isAlive)
        {
            RenderParticle(renderer, particle);
        }
    }

    // Blend modunu normale döndür
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
}

void ParticleEmitter::Emit(int count)
{
    for (int i = 0; i < count; ++i)
    {
        SpawnParticle();
    }
}

void ParticleEmitter::Burst(int count)
{
    Emit(count);
}

void ParticleEmitter::Start()
{
    m_isActive = true;
    m_emissionAccumulator = 0;
}

void ParticleEmitter::Stop()
{
    m_isActive = false;
}

void ParticleEmitter::SetPosition(float x, float y)
{
    m_posX = x;
    m_posY = y;
}

void ParticleEmitter::GetPosition(float &x, float &y) const
{
    x = m_posX;
    y = m_posY;
}

void ParticleEmitter::SetStartColor(Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
    m_startR = r;
    m_startG = g;
    m_startB = b;
    m_startA = a;
}

void ParticleEmitter::SetEndColor(Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
    m_endR = r;
    m_endG = g;
    m_endB = b;
    m_endA = a;
}

int ParticleEmitter::GetActiveParticleCount() const
{
    int count = 0;
    for (const auto &p : m_particles)
    {
        if (p.isAlive)
            count++;
    }
    return count;
}

bool ParticleEmitter::HasAliveParticles() const
{
    for (const auto &p : m_particles)
    {
        if (p.isAlive)
            return true;
    }
    return false;
}

void ParticleEmitter::Clear()
{
    for (auto &p : m_particles)
    {
        p.isAlive = false;
    }
    m_isActive = false;
}

// ==================== Private Fonksiyonlar ====================

Particle *ParticleEmitter::FindDeadParticle()
{
    for (auto &p : m_particles)
    {
        if (!p.isAlive)
        {
            return &p;
        }
    }
    return nullptr; // Havuz dolu
}

void ParticleEmitter::SpawnParticle()
{
    Particle *p = FindDeadParticle();
    if (!p)
        return; // Havuz dolu

    // Parçacığı canlandır
    p->isAlive = true;
    p->age = 0;

    // Konum (yayılma şekline göre)
    switch (m_emissionShape)
    {
    case EmissionShape::POINT:
        p->x = m_posX;
        p->y = m_posY;
        break;

    case EmissionShape::CIRCLE:
    {
        float angle = RandomFloat(0, 360) * (M_PI / 180.0f);
        float radius = RandomFloat(0, m_emissionRadius);
        p->x = m_posX + cos(angle) * radius;
        p->y = m_posY + sin(angle) * radius;
        break;
    }

    case EmissionShape::RECTANGLE:
        p->x = m_posX + RandomFloat(-m_emissionRadius, m_emissionRadius);
        p->y = m_posY + RandomFloat(-m_emissionRadius, m_emissionRadius);
        break;

    case EmissionShape::CONE:
        p->x = m_posX;
        p->y = m_posY;
        break;
    }

    // Hız (yön + büyüklük)
    float speed = RandomFloat(m_minSpeed, m_maxSpeed);
    float angle = m_emissionAngle + RandomFloat(-m_spreadAngle / 2.0f, m_spreadAngle / 2.0f);
    float angleRad = angle * (M_PI / 180.0f);

    p->velocityX = cos(angleRad) * speed;
    p->velocityY = sin(angleRad) * speed;

    // İvme (yerçekimi)
    p->accelerationX = m_gravityX;
    p->accelerationY = m_gravityY;

    // Yaşam süresi
    p->lifetime = RandomFloat(m_minLifetime, m_maxLifetime);

    // Boyut
    p->size = RandomFloat(m_minSize, m_maxSize);
    p->startSize = p->size;

    // Rotasyon
    p->rotation = RandomFloat(0, 360);
    p->rotationSpeed = RandomFloat(m_minRotationSpeed, m_maxRotationSpeed);

    // Renkler
    p->startR = m_startR;
    p->startG = m_startG;
    p->startB = m_startB;
    p->startA = m_startA;
    p->endR = m_endR;
    p->endG = m_endG;
    p->endB = m_endB;
    p->endA = m_endA;
    p->r = m_startR;
    p->g = m_startG;
    p->b = m_startB;
    p->a = m_startA;
}

void ParticleEmitter::UpdateParticle(Particle &p, float deltaTime)
{
    // Yaşı artır
    p.age += deltaTime;

    // Ömrü bittiyse öldür
    if (p.age >= p.lifetime)
    {
        p.isAlive = false;
        return;
    }

    // Fizik güncellemesi
    p.velocityX += p.accelerationX * deltaTime;
    p.velocityY += p.accelerationY * deltaTime;
    p.x += p.velocityX * deltaTime;
    p.y += p.velocityY * deltaTime;

    // Rotasyon güncellemesi
    p.rotation += p.rotationSpeed * deltaTime;

    // Yaşa göre interpolasyon faktörü (0.0 - 1.0)
    float t = p.GetLifePercent();

    // Renk geçişi
    p.r = Lerp(p.startR, p.endR, t);
    p.g = Lerp(p.startG, p.endG, t);
    p.b = Lerp(p.startB, p.endB, t);
    p.a = Lerp(p.startA, p.endA, t);

    // Boyut küçülmesi (opsiyonel - yaşa göre küçülme)
    p.size = p.startSize * (1.0f - t * 0.5f); // %50'ye kadar küçül
}

void ParticleEmitter::RenderParticle(SDL_Renderer *renderer, const Particle &p)
{
    if (m_texture)
    {
        // Texture ile çiz
        SDL_SetTextureAlphaMod(m_texture, p.a);
        SDL_SetTextureColorMod(m_texture, p.r, p.g, p.b);

        SDL_Rect destRect = {
            static_cast<int>(p.x - p.size / 2),
            static_cast<int>(p.y - p.size / 2),
            static_cast<int>(p.size),
            static_cast<int>(p.size)};

        SDL_RenderCopyEx(renderer, m_texture, nullptr, &destRect,
                         p.rotation, nullptr, SDL_FLIP_NONE);
    }
    else
    {
        // Basit dikdörtgen çiz
        SDL_SetRenderDrawColor(renderer, p.r, p.g, p.b, p.a);

        SDL_Rect rect = {
            static_cast<int>(p.x - p.size / 2),
            static_cast<int>(p.y - p.size / 2),
            static_cast<int>(p.size),
            static_cast<int>(p.size)};

        SDL_RenderFillRect(renderer, &rect);
    }
}

float ParticleEmitter::RandomFloat(float min, float max)
{
    return min + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (max - min)));
}

Uint8 ParticleEmitter::Lerp(Uint8 a, Uint8 b, float t)
{
    return static_cast<Uint8>(a + (b - a) * t);
}
