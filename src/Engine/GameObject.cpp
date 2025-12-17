#include "../../include/Engine/GameObject.h"

// Yapıcı: Temel özellikleri başlat
GameObject::GameObject(int x, int y, int width, int height)
    : m_X(x), m_Y(y), m_Width(width), m_Height(height), m_IsActive(true), m_Texture(nullptr)
{
}

// Yıkıcı: Texture temizliği ana Game sınıfında yapılacak
GameObject::~GameObject()
{
    m_Texture = nullptr;
}

// Pozisyonu ayarla
void GameObject::SetPosition(int x, int y)
{
    m_X = x;
    m_Y = y;
}

// SDL_Rect döndür (çarpışma ve çizim için kullanışlı)
SDL_Rect GameObject::GetRect() const
{
    SDL_Rect rect;
    rect.x = m_X;
    rect.y = m_Y;
    rect.w = m_Width;
    rect.h = m_Height;
    return rect;
}
