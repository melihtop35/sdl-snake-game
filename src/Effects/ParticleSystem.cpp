#include "../../include/Effects/ParticleSystem.h"
#include "../../include/Game/Food.h"
#include <algorithm>

ParticleSystem &ParticleSystem::GetInstance()
{
    static ParticleSystem instance;
    return instance;
}

ParticleSystem::ParticleSystem()
    : m_cleanupTimer(0)
{
}

ParticleSystem::~ParticleSystem()
{
    Cleanup();
}

void ParticleSystem::Initialize()
{
    // Gerekirse başlangıç ayarları
    m_cleanupTimer = 0;
}

void ParticleSystem::Update(float deltaTime)
{
    // İsimli emitter'ları güncelle
    for (auto &pair : m_namedEmitters)
    {
        pair.second->Update(deltaTime);
    }

    // One-shot emitter'ları güncelle
    for (auto &emitter : m_oneShotEmitters)
    {
        emitter->Update(deltaTime);
    }

    // Sıralı efektleri güncelle
    UpdateSequencedEffects(deltaTime);

    // Periyodik temizlik
    m_cleanupTimer += deltaTime;
    if (m_cleanupTimer >= CLEANUP_INTERVAL)
    {
        CleanupDeadEmitters();
        m_cleanupTimer = 0;
    }
}

void ParticleSystem::Render(SDL_Renderer *renderer)
{
    // İsimli emitter'ları render et
    for (auto &pair : m_namedEmitters)
    {
        pair.second->Render(renderer);
    }

    // One-shot emitter'ları render et
    for (auto &emitter : m_oneShotEmitters)
    {
        emitter->Render(renderer);
    }
}

void ParticleSystem::Cleanup()
{
    m_namedEmitters.clear();
    m_oneShotEmitters.clear();
}

ParticleEmitter *ParticleSystem::CreateEmitter(const std::string &name, int maxParticles)
{
    auto emitter = std::make_unique<ParticleEmitter>(maxParticles);
    ParticleEmitter *ptr = emitter.get();
    m_namedEmitters[name] = std::move(emitter);
    return ptr;
}

ParticleEmitter *ParticleSystem::GetEmitter(const std::string &name)
{
    auto it = m_namedEmitters.find(name);
    if (it != m_namedEmitters.end())
    {
        return it->second.get();
    }
    return nullptr;
}

void ParticleSystem::RemoveEmitter(const std::string &name)
{
    m_namedEmitters.erase(name);
}

void ParticleSystem::ClearAllEmitters()
{
    m_namedEmitters.clear();
    m_oneShotEmitters.clear();
}

// ==================== Hazır Efekt Preset'leri ====================

void ParticleSystem::PlayExplosion(float x, float y, int particleCount)
{
    SpawnOneShot(x, y,
                 particleCount,
                 100.0f, 300.0f, // Hız
                 0.3f, 0.8f,     // Yaşam süresi
                 255, 200, 50,   // Başlangıç rengi (turuncu/sarı)
                 255, 50, 0,     // Bitiş rengi (kırmızı)
                 200.0f,         // Yerçekimi
                 4.0f, 12.0f);   // Boyut
}

void ParticleSystem::PlaySparkle(float x, float y, int particleCount)
{
    SpawnOneShot(x, y,
                 particleCount,
                 50.0f, 150.0f, // Hız
                 0.2f, 0.5f,    // Yaşam süresi
                 255, 255, 100, // Başlangıç rengi (parlak sarı)
                 255, 255, 255, // Bitiş rengi (beyaz)
                 0.0f,          // Yerçekimi yok
                 2.0f, 6.0f);   // Boyut
}

void ParticleSystem::PlaySmoke(float x, float y)
{
    auto emitter = std::make_unique<ParticleEmitter>(50);
    emitter->SetPosition(x, y);
    emitter->SetEmissionRate(15.0f);
    emitter->SetEmissionShape(EmissionShape::CIRCLE);
    emitter->SetEmissionRadius(5.0f);
    emitter->SetEmissionAngle(-90.0f); // Yukarı
    emitter->SetSpreadAngle(30.0f);
    emitter->SetSpeed(20.0f, 50.0f);
    emitter->SetLifetime(1.0f, 2.0f);
    emitter->SetSize(8.0f, 20.0f);
    emitter->SetStartColor(100, 100, 100, 180);
    emitter->SetEndColor(50, 50, 50, 0);
    emitter->SetGravity(0, -30.0f); // Yukarı yükselme
    emitter->Start();

    m_oneShotEmitters.push_back(std::move(emitter));
}

void ParticleSystem::PlayFirework(float x, float y)
{
    // Çoklu renkli patlama
    SpawnOneShot(x, y, 20,
                 80.0f, 200.0f,
                 0.5f, 1.0f,
                 255, 100, 100, // Kırmızı
                 255, 50, 50,
                 100.0f, 3.0f, 6.0f);

    SpawnOneShot(x, y, 20,
                 80.0f, 200.0f,
                 0.5f, 1.0f,
                 100, 255, 100, // Yeşil
                 50, 255, 50,
                 100.0f, 3.0f, 6.0f);

    SpawnOneShot(x, y, 20,
                 80.0f, 200.0f,
                 0.5f, 1.0f,
                 100, 100, 255, // Mavi
                 50, 50, 255,
                 100.0f, 3.0f, 6.0f);
}

void ParticleSystem::PlayCollect(float x, float y)
{
    // Yem toplama efekti - Yeşil parıltı
    SpawnOneShot(x, y,
                 12,
                 60.0f, 120.0f,
                 0.2f, 0.4f,
                 100, 255, 100, // Yeşil
                 200, 255, 200, // Açık yeşil
                 -50.0f,        // Hafif yukarı
                 3.0f, 7.0f);
}

void ParticleSystem::PlayDeath(float x, float y)
{
    // Ölüm efekti - Kırmızı patlama
    SpawnOneShot(x, y,
                 40,
                 50.0f, 200.0f,
                 0.4f, 1.0f,
                 255, 0, 0, // Kırmızı
                 100, 0, 0, // Koyu kırmızı
                 150.0f,
                 5.0f, 15.0f);
}

void ParticleSystem::SpawnOneShot(float x, float y,
                                  int particleCount,
                                  float minSpeed, float maxSpeed,
                                  float minLifetime, float maxLifetime,
                                  Uint8 startR, Uint8 startG, Uint8 startB,
                                  Uint8 endR, Uint8 endG, Uint8 endB,
                                  float gravity,
                                  float minSize, float maxSize)
{
    auto emitter = std::make_unique<ParticleEmitter>(particleCount);
    emitter->SetPosition(x, y);
    emitter->SetEmissionShape(EmissionShape::POINT);
    emitter->SetSpreadAngle(360.0f); // Her yöne
    emitter->SetSpeed(minSpeed, maxSpeed);
    emitter->SetLifetime(minLifetime, maxLifetime);
    emitter->SetSize(minSize, maxSize);
    emitter->SetStartColor(startR, startG, startB, 255);
    emitter->SetEndColor(endR, endG, endB, 0);
    emitter->SetGravity(0, gravity);
    emitter->SetBlendMode(ParticleBlendMode::ADDITIVE);

    // Anlık burst
    emitter->Burst(particleCount);

    m_oneShotEmitters.push_back(std::move(emitter));
}

int ParticleSystem::GetTotalParticleCount() const
{
    int count = 0;

    for (const auto &pair : m_namedEmitters)
    {
        count += pair.second->GetActiveParticleCount();
    }

    for (const auto &emitter : m_oneShotEmitters)
    {
        count += emitter->GetActiveParticleCount();
    }

    return count;
}

int ParticleSystem::GetActiveEmitterCount() const
{
    return static_cast<int>(m_namedEmitters.size() + m_oneShotEmitters.size());
}

void ParticleSystem::CleanupDeadEmitters()
{
    // Parçacığı kalmayan one-shot emitter'ları sil
    m_oneShotEmitters.erase(
        std::remove_if(m_oneShotEmitters.begin(), m_oneShotEmitters.end(),
                       [](const std::unique_ptr<ParticleEmitter> &emitter)
                       {
                           return !emitter->IsActive() && !emitter->HasAliveParticles();
                       }),
        m_oneShotEmitters.end());
}

// ==================== YENİ EFEKTLER ====================

void ParticleSystem::PlayFoodCollect(float x, float y, int foodType)
{
    // Merkeze offset ekle (hücrenin ortası)
    float centerX = x + 15.0f; // cellSize/2 yaklaşık
    float centerY = y + 15.0f;

    switch (static_cast<FoodType>(foodType))
    {
    case FoodType::APPLE_RED:
        // Kırmızı elma - Kırmızı yuvarlak parçacıklar
        SpawnOneShot(centerX, centerY,
                     35,
                     80.0f, 200.0f,
                     0.4f, 0.8f,
                     255, 50, 50,   // Kırmızı
                     255, 100, 100, // Açık kırmızı
                     60.0f,
                     6.0f, 14.0f);
        break;

    case FoodType::APPLE_GREEN:
        // Yeşil elma - Yeşil yuvarlak parçacıklar
        SpawnOneShot(centerX, centerY,
                     35,
                     80.0f, 200.0f,
                     0.4f, 0.8f,
                     50, 255, 50,   // Yeşil
                     100, 255, 100, // Açık yeşil
                     60.0f,
                     6.0f, 14.0f);
        break;

    case FoodType::OLIEBOL:
        // Oliebol - Kahverengi/altın parçacıklar
        SpawnOneShot(centerX, centerY,
                     40,
                     70.0f, 180.0f,
                     0.5f, 0.9f,
                     200, 150, 50,  // Altın/kahve
                     255, 200, 100, // Açık altın
                     40.0f,
                     7.0f, 16.0f);
        break;

    case FoodType::EASTER_EGG:
        // Sürpriz yumurta - Mor + Sarı karışık parçacıklar
        // Mor parçacıklar
        SpawnOneShot(centerX, centerY,
                     25,
                     100.0f, 220.0f,
                     0.5f, 0.9f,
                     180, 50, 255,  // Mor
                     220, 100, 255, // Açık mor
                     -40.0f,        // Hafif yukarı
                     6.0f, 13.0f);
        // Sarı parçacıklar
        SpawnOneShot(centerX, centerY,
                     25,
                     100.0f, 220.0f,
                     0.5f, 0.9f,
                     255, 220, 50,  // Sarı
                     255, 255, 150, // Açık sarı
                     -40.0f,
                     6.0f, 13.0f);
        break;

    case FoodType::BOMB:
        // Bomba - Siyah + turuncu patlama
        // Siyah duman
        SpawnOneShot(centerX, centerY,
                     45,
                     60.0f, 150.0f,
                     0.6f, 1.2f,
                     40, 40, 40, // Koyu gri/siyah
                     80, 80, 80, // Gri
                     -30.0f,     // Yukarı yükselme
                     8.0f, 18.0f);
        // Turuncu kıvılcım
        SpawnOneShot(centerX, centerY,
                     35,
                     120.0f, 250.0f,
                     0.3f, 0.6f,
                     255, 150, 0, // Turuncu
                     255, 80, 0,  // Koyu turuncu
                     120.0f,
                     5.0f, 12.0f);
        break;
    }
}

void ParticleSystem::PlayBombDamage(float x, float y)
{
    // Yılanın shrink olan parçası için yeşil patlama
    float centerX = x + 15.0f;
    float centerY = y + 15.0f;

    SpawnOneShot(centerX, centerY,
                 50,
                 120.0f, 280.0f,
                 0.5f, 1.0f,
                 0, 255, 0, // Parlak yeşil
                 0, 150, 0, // Koyu yeşil
                 150.0f,    // Aşağı düşme
                 7.0f, 16.0f);
}

void ParticleSystem::PlaySnakeDeathSequence(const std::vector<std::pair<float, float>> &segments, float delay)
{
    if (segments.empty())
        return;

    SequencedEffect effect;
    effect.positions = segments;
    effect.delay = delay;
    effect.timer = 0;
    effect.currentIndex = 0;
    effect.isActive = true;

    m_sequencedEffects.push_back(effect);
}

void ParticleSystem::UpdateSequencedEffects(float deltaTime)
{
    for (auto &effect : m_sequencedEffects)
    {
        if (!effect.isActive)
            continue;

        effect.timer += deltaTime;

        // Sıradaki patlamaları kontrol et
        while (effect.currentIndex < static_cast<int>(effect.positions.size()) &&
               effect.timer >= effect.delay * effect.currentIndex)
        {
            // Bu segmentin patlamasını oynat
            float x = effect.positions[effect.currentIndex].first;
            float y = effect.positions[effect.currentIndex].second;
            float centerX = x + 15.0f;
            float centerY = y + 15.0f;

            // Yeşil segment patlaması
            SpawnOneShot(centerX, centerY,
                         40,
                         100.0f, 220.0f,
                         0.4f, 0.8f,
                         0, 255, 0,   // Parlak yeşil
                         50, 200, 50, // Açık yeşil
                         100.0f,
                         6.0f, 14.0f);

            effect.currentIndex++;
        }

        // Tüm patlamalar bittiyse efekti deaktif et
        if (effect.currentIndex >= static_cast<int>(effect.positions.size()))
        {
            effect.isActive = false;
        }
    }

    // Biten efektleri temizle
    m_sequencedEffects.erase(
        std::remove_if(m_sequencedEffects.begin(), m_sequencedEffects.end(),
                       [](const SequencedEffect &e)
                       { return !e.isActive; }),
        m_sequencedEffects.end());
}
