#pragma once

#include "../Engine/GameObject.h"
#include <random>
#include <vector>

// İleride farklı yem türleri için kullanılabilir
enum class FoodType
{
    APPLE_RED,   // Normal elma - 50%
    APPLE_GREEN, // Normal yeşil elma - 20%
    OLIEBOL,     // Oliebol - 10%
    EASTER_EGG,  // Sürpriz yumurta (buff) - 5%
    BOMB         // Hasar aldıran bomba - 15%
};

class Food : public GameObject
{
private:
    // Puan değeri
    int m_pointValue;

    // Yem türü
    FoodType m_type;

    // Rastgele sayı üreteci
    std::mt19937 m_randomEngine;

    // Grid bilgileri (yem sadece grid hücrelerinde spawn olabilir)
    int m_gridWidth;   // Kaç hücre genişliğinde
    int m_gridHeight;  // Kaç hücre yüksekliğinde
    int m_cellSize;    // Her hücrenin piksel boyutu
    int m_hudOffset;   // HUD için dikey offset
    int m_gridOffsetX; // Grid için yatay offset

    // Bomba zamanlayıcısı
    float m_lifetime;   // Kalan ömür (saniye)
    bool m_hasLifetime; // Bu yemin ömrü var mı?
    float m_blinkTimer; // Yanıp sönme zamanlayıcısı
    bool m_isVisible;   // Görünür mü (yanıp sönme için)

    // Tip bazlı texture'lar
    SDL_Texture *m_textureAppleRed;
    SDL_Texture *m_textureAppleGreen;
    SDL_Texture *m_textureOliebol;
    SDL_Texture *m_textureEasterEgg;
    SDL_Texture *m_textureBomb;

public:
    // Yapıcı
    Food(int cellSize, int gridWidth, int gridHeight, int hudOffset = 50, int gridOffsetX = 0);

    // Yıkıcı
    ~Food();

    // GameObject'ten override
    void Update(float deltaTime) override;
    void Render(SDL_Renderer *renderer) override;

    // Yeniden konumlandırma (rastgele pozisyon)
    void Spawn();

    // Yılanın gövdesinden kaçınarak yeniden konumlan
    // forbiddenPositions: yılanın gövde parçalarının pozisyonları
    void Spawn(const std::vector<std::pair<int, int>> &forbiddenPositions);

    // Rastgele tip ile spawn (yüzdelik oranlar)
    void SpawnWithRandomType();
    void SpawnWithRandomType(const std::vector<std::pair<int, int>> &forbiddenPositions);

    // Getter'lar
    int GetPointValue() const { return m_pointValue; }
    FoodType GetType() const { return m_type; }
    bool IsExpired() const { return m_hasLifetime && m_lifetime <= 0; }
    float GetLifetime() const { return m_lifetime; }

    // Setter'lar
    void SetPointValue(int points) { m_pointValue = points; }
    void SetType(FoodType type);

    // Texture ayarlama
    void SetTextures(SDL_Texture *appleRed, SDL_Texture *appleGreen,
                     SDL_Texture *oliebol, SDL_Texture *easterEgg, SDL_Texture *bomb);

private:
    // Rastgele tip seç (yüzdelik oranlarla)
    FoodType GetRandomType();

    // Mevcut tipe göre texture'ı güncelle
    void UpdateTextureForType();
};
