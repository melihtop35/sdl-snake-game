#pragma once
#include "../Engine/GameObject.h"

class Entity : public GameObject
{
protected:
    // Hız bileşenleri
    float m_velocityX, m_velocityY, m_speed;

public:
    // Yapıcı
    Entity(int x, int y, int width, int height, float speed);
    virtual ~Entity();
    virtual void Update(float deltaTime) override = 0;
    virtual void Render(SDL_Renderer *renderer) override = 0;
    virtual void Move(float dirX, float dirY);

    float GetSpeed() const { return m_speed; }
    float GetVelocity() const { return m_velocityX, m_velocityY; }

    void SetSpeed(float speed) { m_speed = speed; }
    void SetVelocity(float vx, float vy);

    virtual bool CheckCollision(const GameObject &other) const;
};