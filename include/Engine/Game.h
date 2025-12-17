#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>
#include <string>
#include <unordered_map>

class Game
{
private:
    // SDL bileşenleri
    SDL_Window *m_window;
    SDL_Renderer *m_renderer;

    // Oyun durumu
    bool m_isRunning;

    // Pencere ayarları
    int m_windowWidth;
    int m_windowHeight;
    std::string m_windowTitle;

    // Zamanlama
    int m_targetFPS;
    int m_frameDelay; // Milisaniye cinsinden frame süresi
    Uint32 m_lastFrameTime;
    float m_deltaTime; // Son frame'den bu yana geçen süre (saniye)

    // Grid ayarları
    int m_gridWidth;
    int m_gridHeight;
    int m_cellSize;
    int m_gridOffsetX; // Grid'in ekrandaki X ofseti
    int m_gridOffsetY; // Grid'in ekrandaki Y ofseti (HUD yüksekliği)

    // HUD yüksekliği (çözünürlüğe göre dinamik)
    int m_hudHeight;

    // Texture'lar (sprite'lar)
    std::unordered_map<std::string, SDL_Texture *> m_textures;

    // Kaynak dosyaları için temel yol
    std::string m_basePath;

public:
    // Yapıcı ve yıkıcı
    Game();
    ~Game();

    // Ana fonksiyonlar
    bool Initialize(const std::string &title);
    void Run();     // Ana oyun döngüsü
    void Cleanup(); // Kaynakları temizle

    // Durum kontrolü
    bool IsRunning() const { return m_isRunning; }
    void Quit() { m_isRunning = false; }

    // Getter'lar
    SDL_Renderer *GetRenderer() const { return m_renderer; }
    SDL_Window *GetWindow() const { return m_window; }
    int GetWindowWidth() const { return m_windowWidth; }
    int GetWindowHeight() const { return m_windowHeight; }
    float GetDeltaTime() const { return m_deltaTime; }

private:
    // Oyun döngüsü adımları
    void HandleEvents(); // Girdi işleme
    void Update();       // Oyun mantığı güncelleme
    void Render();       // Ekrana çizim

    // Kaynak yönetimi
    bool LoadAssets();   // Dokuları, sesleri yükle
    void UnloadAssets(); // Kaynakları serbest bırak

    // Texture yükleme yardımcısı
    SDL_Texture *LoadTexture(const std::string &path);

    // SDL başlatma yardımcıları
    bool InitSDL();
    bool CreateFullscreenWindow(const std::string &title);
    bool CreateRenderer();

    // Zamanlama
    void CalculateDeltaTime();
    void CapFrameRate();

    // Menü callback'leri
    void SetupUICallbacks();

    // ESC tuşu
    void HandleEscapeKey();

    // Responsive pencere
    void RecalculateGrid();
    void ChangeResolution(int newWidth, int newHeight);
};
