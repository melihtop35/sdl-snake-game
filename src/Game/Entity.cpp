#include "../../include/Game/Entity.h"

// Yapıcı: GameObject'i başlat ve hız özelliklerini ayarla
Entity::Entity(int x, int y, int width, int height, float speed)
    : GameObject(x, y, width, height), m_velocityX(0.0f), m_velocityY(0.0f), m_speed(speed)
{
}

// Yıkıcı
Entity::~Entity()
{
    // Özel temizlik gerekmiyor, üst sınıf halleder
}

// Yön vektörüne göre hareket et
void Entity::Move(float dirX, float dirY)
{
    m_velocityX = dirX * m_speed;
    m_velocityY = dirY * m_speed;
}

// Hız bileşenlerini doğrudan ayarla
void Entity::SetVelocity(float vx, float vy)
{
    m_velocityX = vx;
    m_velocityY = vy;
}

// AABB (Axis-Aligned Bounding Box) çarpışma kontrolü
bool Entity::CheckCollision(const GameObject &other) const
{
    SDL_Rect myRect = GetRect();
    SDL_Rect otherRect = other.GetRect();

    // SDL'in dahili çarpışma fonksiyonunu kullan
    return SDL_HasIntersection(&myRect, &otherRect) == SDL_TRUE;
}
