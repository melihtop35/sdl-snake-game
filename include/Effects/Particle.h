#pragma once
#include <SDL2/SDL.h>

/**
 * @brief Tek bir parçacığı temsil eden yapı
 *
 * Her parçacık kendi konumu, hızı, rengi ve yaşam süresine sahiptir.
 * Object pooling için isAlive flag'i kullanılır.
 */
struct Particle
{
    // Konum
    float x, y;

    // Hız vektörü
    float velocityX, velocityY;

    // İvme (yerçekimi vb.)
    float accelerationX, accelerationY;

    // Renk (RGBA)
    Uint8 r, g, b, a;

    // Başlangıç ve bitiş renkleri (interpolasyon için)
    Uint8 startR, startG, startB, startA;
    Uint8 endR, endG, endB, endA;

    // Boyut
    float size;
    float startSize;

    // Rotasyon
    float rotation;
    float rotationSpeed;

    // Yaşam süresi
    float lifetime; // Toplam ömür (saniye)
    float age;      // Şu anki yaş (saniye)

    // Aktiflik durumu
    bool isAlive;

    /**
     * @brief Parçacığı varsayılan değerlerle başlatır
     */
    Particle()
        : x(0), y(0), velocityX(0), velocityY(0), accelerationX(0), accelerationY(0), r(255), g(255), b(255), a(255), startR(255), startG(255), startB(255), startA(255), endR(255), endG(255), endB(255), endA(0), size(5.0f), startSize(5.0f), rotation(0), rotationSpeed(0), lifetime(1.0f), age(0), isAlive(false)
    {
    }

    /**
     * @brief Yaşam yüzdesini döndürür (0.0 - 1.0)
     */
    float GetLifePercent() const
    {
        return (lifetime > 0) ? (age / lifetime) : 1.0f;
    }

    /**
     * @brief Parçacığı sıfırlar ve yeniden kullanıma hazırlar
     */
    void Reset()
    {
        x = y = 0;
        velocityX = velocityY = 0;
        accelerationX = accelerationY = 0;
        r = g = b = a = 255;
        size = startSize = 5.0f;
        rotation = rotationSpeed = 0;
        lifetime = 1.0f;
        age = 0;
        isAlive = false;
    }
};
