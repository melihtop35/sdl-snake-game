#pragma once

#include "../Game/Player.h"
#include <vector>
#include <SDL2/SDL.h>

// Yılanın her bir parçası için yapı
struct SnakeSegment
{
    int x;
    int y;

    SnakeSegment(int _x = 0, int _y = 0) : x(_x), y(_y) {}
};

// Snake: Player'dan türetilmiş yılan sınıfı
// Yılan oyununa özgü tüm özellikler burada
class Snake : public Player
{
private:
    // Yılanın gövde parçaları (ilk eleman baş)
    std::vector<SnakeSegment> m_Body;

    // Büyüme durumu
    bool m_ShouldGrow;

    // Her segment'in piksel boyutu
    int m_SegmentSize;

    // Farklı parçalar için dokular
    SDL_Texture *m_HeadTexture;
    SDL_Texture *m_BodyTexture;
    SDL_Texture *m_TailTexture;

public:
    // Yapıcı: başlangıç pozisyonu ve segment boyutu
    Snake(int startX, int startY, int segmentSize);

    // Yıkıcı
    ~Snake();

    // Player'dan override edilen fonksiyonlar
    void Update(float deltaTime) override;
    void Render(SDL_Renderer *renderer) override;

    // Player'ın Reset metodunu override et
    void Reset(int x, int y) override;

    // Büyüme
    void Grow() { m_ShouldGrow = true; }

    // Küçülme (bomba yenildiğinde)
    void Shrink(int amount = 1);

    // Getter'lar
    const std::vector<SnakeSegment> &GetBody() const { return m_Body; }
    SnakeSegment GetHeadPosition() const;
    int GetSegmentSize() const { return m_SegmentSize; }
    int GetLength() const { return static_cast<int>(m_Body.size()); }

    // Yılana özgü çarpışma kontrolleri
    bool CheckSelfCollision() const;
    bool CheckCollisionWithPoint(int x, int y) const;

    // Dokuları ayarla
    void SetTextures(SDL_Texture *head, SDL_Texture *body, SDL_Texture *tail);

private:
    // Dahili hareket fonksiyonu
    void MoveSnake();
};