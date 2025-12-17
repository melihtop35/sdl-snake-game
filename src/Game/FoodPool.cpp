#include "../../include/Game/FoodPool.h"
#include <chrono>
#include <algorithm>

// Yapıcı
FoodPool::FoodPool()
    : m_maxActiveFoods(10),
      m_spawnTimer(0.0f),
      m_spawnInterval(3.0f),
      m_minSpawnInterval(2.0f),
      m_maxSpawnInterval(5.0f),
      m_gridWidth(0),
      m_gridHeight(0),
      m_cellSize(0),
      m_hudOffset(50),
      m_gridOffsetX(0),
      m_textureAppleRed(nullptr),
      m_textureAppleGreen(nullptr),
      m_textureOliebol(nullptr),
      m_textureEasterEgg(nullptr),
      m_textureBomb(nullptr),
      m_onFoodSpawn(nullptr)
{
    // Rastgele sayı üretecini başlat
    auto seed = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    m_randomEngine = std::mt19937(static_cast<unsigned int>(seed));
}

// Yıkıcı
FoodPool::~FoodPool()
{
    m_pool.clear();
}

// Başlatma
void FoodPool::Initialize(int gridWidth, int gridHeight, int cellSize,
                          int hudOffset, int gridOffsetX,
                          int poolSize, int maxActive)
{
    m_gridWidth = gridWidth;
    m_gridHeight = gridHeight;
    m_cellSize = cellSize;
    m_hudOffset = hudOffset;
    m_gridOffsetX = gridOffsetX;
    m_maxActiveFoods = maxActive;

    // Pool'u ön-tahsis et
    m_pool.clear();
    m_pool.reserve(poolSize);

    for (int i = 0; i < poolSize; i++)
    {
        auto food = std::make_unique<Food>(cellSize, gridWidth, gridHeight, hudOffset, gridOffsetX);
        food->SetActive(false); // Başlangıçta hepsi inaktif

        // Texture'ları ayarla
        if (m_textureAppleRed || m_textureAppleGreen || m_textureOliebol ||
            m_textureEasterEgg || m_textureBomb)
        {
            food->SetTextures(m_textureAppleRed, m_textureAppleGreen,
                              m_textureOliebol, m_textureEasterEgg, m_textureBomb);
        }

        m_pool.push_back(std::move(food));
    }

    // İlk spawn aralığını rastgele seç
    RandomizeSpawnInterval();
    m_spawnTimer = 0.0f; // Hemen ilk spawn için
}

// Güncelleme
void FoodPool::Update(float deltaTime, const std::vector<std::pair<int, int>> &forbiddenPositions)
{
    // Aktif food'ları güncelle
    for (auto &food : m_pool)
    {
        if (food->IsActive())
        {
            food->Update(deltaTime);

            // Süresi dolan food'ları deaktif et
            if (food->IsExpired())
            {
                food->SetActive(false);
            }
        }
    }

    // Spawn zamanlayıcı
    int activeCount = GetActiveFoodCount();
    if (activeCount < m_maxActiveFoods)
    {
        m_spawnTimer += deltaTime;

        if (m_spawnTimer >= m_spawnInterval)
        {
            // Yeni food spawn et
            Food *newFood = SpawnFood(forbiddenPositions);

            if (newFood && m_onFoodSpawn)
            {
                m_onFoodSpawn(newFood);
            }

            m_spawnTimer = 0.0f;
            RandomizeSpawnInterval();
        }
    }
}

// Render
void FoodPool::Render(SDL_Renderer *renderer)
{
    for (auto &food : m_pool)
    {
        if (food->IsActive())
        {
            food->Render(renderer);
        }
    }
}

// Food spawn et
Food *FoodPool::SpawnFood(const std::vector<std::pair<int, int>> &forbiddenPositions)
{
    // Aktif sayısı kontrolü
    if (GetActiveFoodCount() >= m_maxActiveFoods)
    {
        return nullptr;
    }

    // Pool'dan uygun food al
    Food *food = GetFromPool();
    if (!food)
    {
        return nullptr;
    }

    // Rastgele tip ile spawn et
    food->SpawnWithRandomType(forbiddenPositions);

    return food;
}

// Belirli bir food'u deaktif et
void FoodPool::DeactivateFood(Food *food)
{
    if (food)
    {
        food->SetActive(false);
    }
}

// Tüm aktif food'ları al
std::vector<Food *> FoodPool::GetActiveFoods()
{
    std::vector<Food *> activeFoods;
    for (auto &food : m_pool)
    {
        if (food->IsActive())
        {
            activeFoods.push_back(food.get());
        }
    }
    return activeFoods;
}

// Aktif food sayısı
int FoodPool::GetActiveFoodCount() const
{
    int count = 0;
    for (const auto &food : m_pool)
    {
        if (food->IsActive())
        {
            count++;
        }
    }
    return count;
}

// Belirli pozisyondaki food'u bul
Food *FoodPool::GetFoodAt(int x, int y)
{
    for (auto &food : m_pool)
    {
        if (food->IsActive() && food->GetX() == x && food->GetY() == y)
        {
            return food.get();
        }
    }
    return nullptr;
}

// Tüm food'ları deaktif et
void FoodPool::DeactivateAll()
{
    for (auto &food : m_pool)
    {
        food->SetActive(false);
    }
    m_spawnTimer = 0.0f;
}

// Spawn aralığını ayarla
void FoodPool::SetSpawnInterval(float min, float max)
{
    m_minSpawnInterval = min;
    m_maxSpawnInterval = max;
    RandomizeSpawnInterval();
}

// Texture ayarlama
void FoodPool::SetFoodTextures(SDL_Texture *appleRed, SDL_Texture *appleGreen,
                               SDL_Texture *oliebol, SDL_Texture *easterEgg, SDL_Texture *bomb)
{
    m_textureAppleRed = appleRed;
    m_textureAppleGreen = appleGreen;
    m_textureOliebol = oliebol;
    m_textureEasterEgg = easterEgg;
    m_textureBomb = bomb;

    // Mevcut food'lara da uygula
    for (auto &food : m_pool)
    {
        food->SetTextures(appleRed, appleGreen, oliebol, easterEgg, bomb);
    }
}

// Grid güncelleme
void FoodPool::OnResize(int newGridWidth, int newGridHeight, int newCellSize, int newGridOffsetX, int newHudOffset)
{
    m_gridWidth = newGridWidth;
    m_gridHeight = newGridHeight;
    m_cellSize = newCellSize;
    m_gridOffsetX = newGridOffsetX;
    m_hudOffset = newHudOffset;

    // Tüm food'ları deaktif et - yeni boyutlarla yeniden spawn olacaklar
    DeactivateAll();

    // Pool'daki food'ları yeniden oluştur (grid değişti)
    int poolSize = static_cast<int>(m_pool.size());
    m_pool.clear();

    for (int i = 0; i < poolSize; i++)
    {
        auto food = std::make_unique<Food>(m_cellSize, m_gridWidth, m_gridHeight, m_hudOffset, m_gridOffsetX);
        food->SetActive(false);

        if (m_textureAppleRed || m_textureAppleGreen || m_textureOliebol ||
            m_textureEasterEgg || m_textureBomb)
        {
            food->SetTextures(m_textureAppleRed, m_textureAppleGreen,
                              m_textureOliebol, m_textureEasterEgg, m_textureBomb);
        }

        m_pool.push_back(std::move(food));
    }
}

// Pool'dan uygun food al
Food *FoodPool::GetFromPool()
{
    // Önce inaktif food ara
    for (auto &food : m_pool)
    {
        if (!food->IsActive())
        {
            return food.get();
        }
    }

    // Hiç inaktif yoksa yeni oluştur ve pool'a ekle
    auto newFood = std::make_unique<Food>(m_cellSize, m_gridWidth, m_gridHeight, m_hudOffset, m_gridOffsetX);
    newFood->SetActive(false);

    if (m_textureAppleRed || m_textureAppleGreen || m_textureOliebol ||
        m_textureEasterEgg || m_textureBomb)
    {
        newFood->SetTextures(m_textureAppleRed, m_textureAppleGreen,
                             m_textureOliebol, m_textureEasterEgg, m_textureBomb);
    }

    Food *ptr = newFood.get();
    m_pool.push_back(std::move(newFood));

    return ptr;
}

// Yeni spawn aralığı hesapla
void FoodPool::RandomizeSpawnInterval()
{
    std::uniform_real_distribution<float> dist(m_minSpawnInterval, m_maxSpawnInterval);
    m_spawnInterval = dist(m_randomEngine);
}
