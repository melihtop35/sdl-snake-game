#pragma once

#include "../Game/Entity.h"
#include "../Game/Direction.h"
#include <SDL2/SDL.h>

// Player: Kullanıcı tarafından kontrol edilen tüm entity'ler için temel sınıf
// Snake, Karakter, Araç vb. bu sınıftan türetilebilir
class Player : public Entity
{
protected:
    // Yön bilgisi - türetilen sınıfların erişebilmesi için protected
    Direction m_currentDirection;
    Direction m_nextDirection;

    // Hareket zamanlaması
    float m_moveTimer;
    float m_moveDelay; // Saniye cinsinden hareket aralığı

public:
    Player(int x, int y, int width, int height, float speed);
    virtual ~Player();

    void Update(float deltaTime) override;
    void Render(SDL_Renderer *renderer) override;

    // Yön kontrolü
    void SetDirection(Direction dir);
    Direction GetDirection() const { return m_currentDirection; }

    // Pozisyonu sıfırla
    virtual void Reset(int x, int y);

    // Hareket hızını ayarla
    void SetMoveDelay(float delay) { m_moveDelay = delay; }
    float GetMoveDelay() const { return m_moveDelay; }
};