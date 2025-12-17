#pragma once
#include <SDL2/SDL.h>

class GameObject
{
protected:
    int m_X, m_Y, m_Width, m_Height;
    bool m_IsActive;
    SDL_Texture *m_Texture;

public:
    GameObject(int x, int y, int width, int height);
    virtual ~GameObject();
    virtual void Update(float deltaTime) = 0;
    virtual void Render(SDL_Renderer *renderer) = 0;
    // Pozisyonu referans parametrelerine atar
    void GetPosition(int &x, int &y) const
    {
        x = m_X;
        y = m_Y;
    }

    // Boyutu referans parametrelerine atar
    void GetSize(int &width, int &height) const
    {
        width = m_Width;
        height = m_Height;
    }

    // Tek değer getter'ları
    int GetX() const { return m_X; }
    int GetY() const { return m_Y; }
    int GetWidth() const { return m_Width; }
    int GetHeight() const { return m_Height; }
    bool IsActive() const { return m_IsActive; };

    void SetPosition(int x, int y);
    void SetActive(bool active) { m_IsActive = active; }

    SDL_Rect GetRect() const;

    void SetTexture(SDL_Texture *texture) { m_Texture = texture; }
};