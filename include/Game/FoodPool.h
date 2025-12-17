#pragma once

#include "Food.h"
#include <vector>
#include <memory>
#include <functional>

// Food spawn event callback
using FoodSpawnCallback = std::function<void(Food *)>;

class FoodPool
{
private:
    // Object pool - tüm food'lar burada saklanır
    std::vector<std::unique_ptr<Food>> m_pool;

    // Aktif food sayısı limiti
    int m_maxActiveFoods;

    // Spawn zamanlayıcı
    float m_spawnTimer;
    float m_spawnInterval;    // Ne sıklıkla yeni food spawn olacak (saniye)
    float m_minSpawnInterval; // Minimum spawn aralığı
    float m_maxSpawnInterval; // Maksimum spawn aralığı

    // Grid bilgileri
    int m_gridWidth;
    int m_gridHeight;
    int m_cellSize;
    int m_hudOffset;
    int m_gridOffsetX;

    // Texture'lar
    SDL_Texture *m_textureAppleRed;
    SDL_Texture *m_textureAppleGreen;
    SDL_Texture *m_textureOliebol;
    SDL_Texture *m_textureEasterEgg;
    SDL_Texture *m_textureBomb;

    // Rastgele sayı üreteci
    std::mt19937 m_randomEngine;

    // Spawn callback
    FoodSpawnCallback m_onFoodSpawn;

public:
    FoodPool();
    ~FoodPool();

    // Başlatma
    void Initialize(int gridWidth, int gridHeight, int cellSize,
                    int hudOffset = 50, int gridOffsetX = 0,
                    int poolSize = 10, int maxActive = 10);

    // Güncelleme - spawn zamanlayıcı ve food'ları güncelle
    void Update(float deltaTime, const std::vector<std::pair<int, int>> &forbiddenPositions);

    // Render
    void Render(SDL_Renderer *renderer);

    // Food spawn et (pool'dan al veya yeni oluştur)
    Food *SpawnFood(const std::vector<std::pair<int, int>> &forbiddenPositions);

    // Belirli bir food'u deaktif et (yenildiğinde)
    void DeactivateFood(Food *food);

    // Tüm aktif food'ları al
    std::vector<Food *> GetActiveFoods();

    // Aktif food sayısı
    int GetActiveFoodCount() const;

    // Belirli pozisyondaki food'u bul (çarpışma için)
    Food *GetFoodAt(int x, int y);

    // Tüm food'ları deaktif et (oyun sıfırlandığında)
    void DeactivateAll();

    // Ayarlar
    void SetMaxActiveFoods(int max) { m_maxActiveFoods = max; }
    void SetSpawnInterval(float min, float max);

    // Texture ayarlama
    void SetFoodTextures(SDL_Texture *appleRed, SDL_Texture *appleGreen,
                         SDL_Texture *oliebol, SDL_Texture *easterEgg, SDL_Texture *bomb);

    // Callback
    void SetFoodSpawnCallback(FoodSpawnCallback callback) { m_onFoodSpawn = callback; }

    // Grid güncelleme (çözünürlük değiştiğinde)
    void OnResize(int newGridWidth, int newGridHeight, int newCellSize, int newGridOffsetX, int newHudOffset);

private:
    // Pool'dan uygun food al veya yeni oluştur
    Food *GetFromPool();

    // Yeni spawn aralığı hesapla
    void RandomizeSpawnInterval();
};
