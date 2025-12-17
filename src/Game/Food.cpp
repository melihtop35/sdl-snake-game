#include "../../include/Game/Food.h"
#include <chrono>

// Yapıcı: GameObject'i başlat ve rastgele üreteci kur
Food::Food(int cellSize, int gridWidth, int gridHeight, int hudOffset, int gridOffsetX)
    : GameObject(0, 0, cellSize, cellSize), m_pointValue(10), m_type(FoodType::APPLE_RED), m_gridWidth(gridWidth), m_gridHeight(gridHeight), m_cellSize(cellSize), m_hudOffset(hudOffset), m_gridOffsetX(gridOffsetX), m_lifetime(0.0f), m_hasLifetime(false), m_blinkTimer(0.0f), m_isVisible(true),
      m_textureAppleRed(nullptr), m_textureAppleGreen(nullptr), m_textureOliebol(nullptr), m_textureEasterEgg(nullptr), m_textureBomb(nullptr)
{
    // Rastgele sayı üretecini şu anki zamanla başlat (seed)
    auto seed = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    m_randomEngine = std::mt19937(static_cast<unsigned int>(seed));

    // İlk spawn (rastgele tip ile)
    SpawnWithRandomType();
}

// Yıkıcı
Food::~Food()
{
    // Özel temizlik gerekmiyor
}

// Güncelleme
void Food::Update(float deltaTime)
{
    // Bomba zamanlayıcısı
    if (m_hasLifetime && m_IsActive)
    {
        m_lifetime -= deltaTime;

        // Son 3 saniyede yanıp sön
        if (m_lifetime <= 3.0f)
        {
            m_blinkTimer += deltaTime;
            // Her 0.2 saniyede yanıp sön
            if (m_blinkTimer >= 0.2f)
            {
                m_isVisible = !m_isVisible;
                m_blinkTimer = 0.0f;
            }
        }

        // Süre doldu mu?
        if (m_lifetime <= 0)
        {
            SetActive(false);
        }
    }
}

// Ekrana çiz
void Food::Render(SDL_Renderer *renderer)
{
    if (!m_IsActive)
        return;

    // Yanıp sönme kontrolü
    if (!m_isVisible && m_hasLifetime && m_lifetime <= 3.0f)
        return;

    SDL_Rect destRect = GetRect();

    if (m_Texture != nullptr)
    {
        SDL_RenderCopy(renderer, m_Texture, nullptr, &destRect);
    }
    else
    {
        // Texture yoksa yem tipine göre renk seç
        switch (m_type)
        {
        case FoodType::APPLE_RED:
            SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255); // Kırmızı
            break;
        case FoodType::APPLE_GREEN:
            SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255); // Yeşil
            break;
        case FoodType::OLIEBOL:
            SDL_SetRenderDrawColor(renderer, 255, 215, 0, 255); // Altın
            break;
        case FoodType::EASTER_EGG:
            SDL_SetRenderDrawColor(renderer, 255, 105, 180, 255); // Pembe
            break;
        case FoodType::BOMB:
            SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255); // Koyu gri
            break;
        }
        SDL_RenderFillRect(renderer, &destRect);

        // Sınır çiz (tip bazlı)
        switch (m_type)
        {
        case FoodType::BOMB:
            SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255); // Kırmızı sınır
            break;
        case FoodType::EASTER_EGG:
            SDL_SetRenderDrawColor(renderer, 148, 0, 211, 255); // Mor sınır
            break;
        default:
            SDL_SetRenderDrawColor(renderer, 139, 0, 0, 255); // Koyu kırmızı
            break;
        }
        SDL_RenderDrawRect(renderer, &destRect);
    }
}

// Rastgele pozisyona spawn ol
void Food::Spawn()
{
    // Grid sınırları içinde rastgele pozisyon
    std::uniform_int_distribution<int> xDist(0, m_gridWidth - 1);
    std::uniform_int_distribution<int> yDist(0, m_gridHeight - 1);

    int gridX = xDist(m_randomEngine);
    int gridY = yDist(m_randomEngine);

    // Grid pozisyonunu piksel pozisyonuna çevir (offset'ler dahil)
    SetPosition(gridX * m_cellSize + m_gridOffsetX, gridY * m_cellSize + m_hudOffset);
    SetActive(true);

    // Görünürlüğü sıfırla
    m_isVisible = true;
    m_blinkTimer = 0.0f;
}

// Rastgele tip ile spawn
void Food::SpawnWithRandomType()
{
    FoodType newType = GetRandomType();
    SetType(newType);
    Spawn();
}

// Yasak pozisyonlardan kaçınarak rastgele tip ile spawn
void Food::SpawnWithRandomType(const std::vector<std::pair<int, int>> &forbiddenPositions)
{
    FoodType newType = GetRandomType();
    SetType(newType);
    Spawn(forbiddenPositions);
}

// Rastgele tip seç (yüzdelik oranlarla)
// 50% APPLE_RED, 20% APPLE_GREEN, 10% OLIEBOL, 5% EASTER_EGG, 15% BOMB
FoodType Food::GetRandomType()
{
    std::uniform_int_distribution<int> dist(1, 100);
    int roll = dist(m_randomEngine);

    if (roll <= 50) // 1-50 (50%)
        return FoodType::APPLE_RED;
    else if (roll <= 70) // 51-70 (20%)
        return FoodType::APPLE_GREEN;
    else if (roll <= 80) // 71-80 (10%)
        return FoodType::OLIEBOL;
    else if (roll <= 85) // 81-85 (5%)
        return FoodType::EASTER_EGG;
    else // 86-100 (15%)
        return FoodType::BOMB;
}

// Yasak pozisyonlardan kaçınarak spawn ol
void Food::Spawn(const std::vector<std::pair<int, int>> &forbiddenPositions)
{
    const int maxAttempts = 100;
    int attempts = 0;

    do
    {
        // Rastgele pozisyon dene
        Spawn();

        // Yasak pozisyonlarla çakışma kontrolü
        bool collision = false;
        for (const auto &pos : forbiddenPositions)
        {
            if (m_X == pos.first && m_Y == pos.second)
            {
                collision = true;
                break;
            }
        }

        // Çakışma yoksa döngüden çık
        if (!collision)
        {
            break;
        }

        attempts++;
    } while (attempts < maxAttempts);

    // maxAttempts'e ulaşıldıysa son pozisyonda kal
    // (grid tamamen doluysa bu olabilir)
}

// Yem tipini ayarla
void Food::SetType(FoodType type)
{
    m_type = type;
    m_hasLifetime = false;
    m_lifetime = 0.0f;

    // Tipe göre puan değerini ayarla
    switch (type)
    {
    case FoodType::APPLE_RED:
        m_pointValue = 10;
        break;
    case FoodType::APPLE_GREEN:
        m_pointValue = 15;
        break;
    case FoodType::OLIEBOL:
        m_pointValue = 50;
        break;
    case FoodType::EASTER_EGG:
        m_pointValue = 100; // Buff + puan
        break;
    case FoodType::BOMB:
        m_pointValue = -20; // Puan -20, hasar var
        // 5-10 saniye arası rastgele ömür
        {
            std::uniform_real_distribution<float> lifeDist(5.0f, 10.0f);
            m_lifetime = lifeDist(m_randomEngine);
            m_hasLifetime = true;
        }
        break;
    }

    // Texture'ı güncelle
    UpdateTextureForType();
}

// Tüm texture'ları ayarla
void Food::SetTextures(SDL_Texture *appleRed, SDL_Texture *appleGreen,
                       SDL_Texture *oliebol, SDL_Texture *easterEgg, SDL_Texture *bomb)
{
    m_textureAppleRed = appleRed;
    m_textureAppleGreen = appleGreen;
    m_textureOliebol = oliebol;
    m_textureEasterEgg = easterEgg;
    m_textureBomb = bomb;

    // Mevcut tipe göre texture'ı güncelle
    UpdateTextureForType();
}

// Mevcut tipe göre texture'ı güncelle
void Food::UpdateTextureForType()
{
    switch (m_type)
    {
    case FoodType::APPLE_RED:
        m_Texture = m_textureAppleRed;
        break;
    case FoodType::APPLE_GREEN:
        m_Texture = m_textureAppleGreen;
        break;
    case FoodType::OLIEBOL:
        m_Texture = m_textureOliebol;
        break;
    case FoodType::EASTER_EGG:
        m_Texture = m_textureEasterEgg;
        break;
    case FoodType::BOMB:
        m_Texture = m_textureBomb;
        break;
    }
}
