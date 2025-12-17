#include "../../include/Game/Player.h"

// Yapıcı: Entity'yi başlat ve Player'a özgü değerleri ayarla
Player::Player(int x, int y, int width, int height, float speed)
    : Entity(x, y, width, height, speed), m_currentDirection(Direction::NONE), m_nextDirection(Direction::NONE), m_moveTimer(0.0f), m_moveDelay(0.005f) // Varsayılan: 5ms hareket gecikmesi
{
}

// Yıkıcı
Player::~Player()
{
    // Özel temizlik gerekmiyor
}

// Her frame çağrılır - hareket zamanlayıcısını yönetir
void Player::Update(float deltaTime)
{
    // Hareket zamanlayıcısını güncelle
    m_moveTimer += deltaTime;

    // Hareket zamanı geldiyse sonraki yönü mevcut yön yap
    if (m_moveTimer >= m_moveDelay)
    {
        m_currentDirection = m_nextDirection;
        m_moveTimer = 0.0f;
    }
}

// Temel render - sadece texture'ı çiz
void Player::Render(SDL_Renderer *renderer)
{
    if (m_Texture != nullptr && m_IsActive)
    {
        SDL_Rect destRect = GetRect();
        SDL_RenderCopy(renderer, m_Texture, nullptr, &destRect);
    }
}

// Yön ayarla - ters yöne gitmeyi engelle
void Player::SetDirection(Direction dir)
{
    // Ters yöne gitmeyi engelle (yılan kendine dönemez)
    bool canChange = false;

    switch (dir)
    {
    case Direction::UP:
        canChange = (m_currentDirection != Direction::DOWN);
        break;
    case Direction::DOWN:
        canChange = (m_currentDirection != Direction::UP);
        break;
    case Direction::LEFT:
        canChange = (m_currentDirection != Direction::RIGHT);
        break;
    case Direction::RIGHT:
        canChange = (m_currentDirection != Direction::LEFT);
        break;
    case Direction::NONE:
        canChange = true;
        break;
    }

    // Eğer henüz hareket etmiyorsa (NONE), her yöne gidebilir
    if (m_currentDirection == Direction::NONE)
    {
        canChange = true;
    }

    if (canChange)
    {
        m_nextDirection = dir;
    }
}

// Pozisyonu ve durumu sıfırla
void Player::Reset(int x, int y)
{
    SetPosition(x, y);
    m_currentDirection = Direction::NONE;
    m_nextDirection = Direction::NONE;
    m_moveTimer = 0.0f;
}
