#include "../../include/Game/Snake.h"

// Yapıcı: Player'ı başlat ve yılanı oluştur
Snake::Snake(int startX, int startY, int segmentSize)
    : Player(startX, startY, segmentSize, segmentSize, 1.0f), m_SegmentSize(segmentSize), m_ShouldGrow(false), m_HeadTexture(nullptr), m_BodyTexture(nullptr), m_TailTexture(nullptr)
{
    // Başlangıçta 3 segmentli yılan oluştur (baş + 2 gövde)
    for (int i = 0; i < 3; i++)
    {
        m_Body.push_back(SnakeSegment(startX - i * segmentSize, startY));
    }
}

// Yıkıcı
Snake::~Snake()
{
    m_Body.clear();
}

// Her frame çağrılır
void Snake::Update(float deltaTime)
{
    // Player'ın Update'ini çağır (zamanlayıcı ve yön yönetimi)
    Player::Update(deltaTime);

    // Hareket zamanı geldiyse ve bir yön varsa hareket et
    // Not: Player::Update içinde m_moveTimer sıfırlanıyor,
    // bu yüzden burada kontrol farklı yapılmalı
    if (m_currentDirection != Direction::NONE)
    {
        // Hareket zamanlayıcısı m_moveDelay'e ulaştığında hareket et
        // Player::Update zaten bunu kontrol ediyor ve m_moveTimer'ı sıfırlıyor
        // Dolayısıyla her sıfırlamada hareket etmeliyiz
        static float lastMoveTimer = 0.0f;
        if (m_moveTimer < lastMoveTimer) // Sıfırlandıysa
        {
            MoveSnake();
        }
        lastMoveTimer = m_moveTimer;
    }
}

// Yılanı bir adım hareket ettir
void Snake::MoveSnake()
{
    if (m_Body.empty())
        return;

    // Mevcut baş pozisyonu
    int newHeadX = m_Body[0].x;
    int newHeadY = m_Body[0].y;

    // Yöne göre yeni baş pozisyonunu hesapla
    switch (m_currentDirection)
    {
    case Direction::UP:
        newHeadY -= m_SegmentSize;
        break;
    case Direction::DOWN:
        newHeadY += m_SegmentSize;
        break;
    case Direction::LEFT:
        newHeadX -= m_SegmentSize;
        break;
    case Direction::RIGHT:
        newHeadX += m_SegmentSize;
        break;
    case Direction::NONE:
        return; // Hareket yok
    }

    // Eğer büyümeyecekse kuyruğu sil
    if (!m_ShouldGrow)
    {
        m_Body.pop_back();
    }
    else
    {
        m_ShouldGrow = false;
    }

    // Yeni başı en öne ekle
    m_Body.insert(m_Body.begin(), SnakeSegment(newHeadX, newHeadY));

    // GameObject'in pozisyonunu güncelle (baş pozisyonu)
    SetPosition(newHeadX, newHeadY);
}

// Yılanı ekrana çiz
void Snake::Render(SDL_Renderer *renderer)
{
    for (size_t i = 0; i < m_Body.size(); i++)
    {
        SDL_Rect destRect;
        destRect.x = m_Body[i].x;
        destRect.y = m_Body[i].y;
        destRect.w = m_SegmentSize;
        destRect.h = m_SegmentSize;

        SDL_Texture *texture = nullptr;
        double angle = 0.0;

        if (i == 0) // Baş
        {
            texture = m_HeadTexture;
            // Yöne göre döndürme açısı
            switch (m_currentDirection)
            {
            case Direction::UP:
                angle = 0.0;
                break;
            case Direction::DOWN:
                angle = 180.0;
                break;
            case Direction::LEFT:
                angle = 270.0;
                break;
            case Direction::RIGHT:
                angle = 90.0;
                break;
            default:
                angle = 0.0;
                break;
            }
        }
        else if (i == m_Body.size() - 1) // Kuyruk
        {
            texture = m_TailTexture;
        }
        else // Gövde
        {
            texture = m_BodyTexture;
        }

        if (texture != nullptr)
        {
            if (i == 0) // Baş için döndürme uygula
            {
                SDL_RenderCopyEx(renderer, texture, nullptr, &destRect,
                                 angle, nullptr, SDL_FLIP_NONE);
            }
            else
            {
                SDL_RenderCopy(renderer, texture, nullptr, &destRect);
            }
        }
        else
        {
            // Texture yoksa renkli kareler çiz
            if (i == 0)
            {
                SDL_SetRenderDrawColor(renderer, 0, 200, 0, 255); // Koyu yeşil baş
            }
            else
            {
                SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255); // Açık yeşil gövde
            }
            SDL_RenderFillRect(renderer, &destRect);

            // Sınır çiz
            SDL_SetRenderDrawColor(renderer, 0, 100, 0, 255);
            SDL_RenderDrawRect(renderer, &destRect);
        }
    }
}

// Yılanı sıfırla
void Snake::Reset(int x, int y)
{
    // Player'ın Reset'ini çağır
    Player::Reset(x, y);

    // Gövdeyi temizle
    m_Body.clear();
    m_ShouldGrow = false;

    // 3 segmentli yılan oluştur
    for (int i = 0; i < 3; i++)
    {
        m_Body.push_back(SnakeSegment(x - i * m_SegmentSize, y));
    }
}

// Baş pozisyonunu döndür
SnakeSegment Snake::GetHeadPosition() const
{
    if (!m_Body.empty())
    {
        return m_Body[0];
    }
    return SnakeSegment(0, 0);
}

// Kendine çarpma kontrolü
bool Snake::CheckSelfCollision() const
{
    // En az 4 segment olmalı kendine çarpabilmesi için
    if (m_Body.size() < 4)
    {
        return false;
    }

    const SnakeSegment &head = m_Body[0];

    // Baş hariç tüm segmentleri kontrol et
    for (size_t i = 1; i < m_Body.size(); i++)
    {
        if (head.x == m_Body[i].x && head.y == m_Body[i].y)
        {
            return true;
        }
    }

    return false;
}

// Belirli bir noktayla çarpışma kontrolü
bool Snake::CheckCollisionWithPoint(int x, int y) const
{
    for (const auto &segment : m_Body)
    {
        if (segment.x == x && segment.y == y)
        {
            return true;
        }
    }
    return false;
}

// Dokuları ayarla
void Snake::SetTextures(SDL_Texture *head, SDL_Texture *body, SDL_Texture *tail)
{
    m_HeadTexture = head;
    m_BodyTexture = body;
    m_TailTexture = tail;
}

// Yılanı küçült (bomba yenildiğinde)
void Snake::Shrink(int amount)
{
    // En az 1 segment (baş) kalmalı
    for (int i = 0; i < amount && m_Body.size() > 1; i++)
    {
        m_Body.pop_back();
    }
}
