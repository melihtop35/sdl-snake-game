#pragma once
#include <SDL2/SDL.h>
#include <string>
#include <vector>
#include <functional>

// Çözünürlük yapısı
struct Resolution
{
    int width;
    int height;
    std::string label;

    Resolution(int w, int h) : width(w), height(h)
    {
        label = std::to_string(w) + "x" + std::to_string(h);
    }
};

// Menü durumları
enum class MenuState
{
    MAIN_MENU,
    SETTINGS,
    PAUSE_MENU,
    GAME_OVER
};

class UIManager
{
private:
    // Ekran boyutları
    int m_screenWidth;
    int m_screenHeight;

    // Native (monitör) çözünürlük
    int m_nativeWidth;
    int m_nativeHeight;
    bool m_isFullscreen;

    // Menü sistemi
    MenuState m_currentMenu;

    // Çözünürlük sistemi
    std::vector<Resolution> m_resolutions;
    int m_currentResolutionIndex;
    std::function<void(int, int)> m_onResolutionChange;

    // Ayarlar
    int m_musicVolume;
    int m_sfxVolume;
    bool m_musicEnabled;
    bool m_sfxEnabled;

    // ImGui durumları
    bool m_imguiInitialized;

    // Pause menüsünden mi gelindi (çözünürlük değiştirme kapatılır)
    bool m_cameFromPause;

    // Menü seçimleri (callback'ler için)
    std::function<void()> m_onStartGame;
    std::function<void()> m_onResumeGame;
    std::function<void()> m_onReturnToMenu;
    std::function<void()> m_onQuitGame;

    UIManager();
    UIManager(const UIManager &) = delete;
    UIManager &operator=(const UIManager &) = delete;

public:
    static UIManager &GetInstance();
    ~UIManager();

    bool Initialize(SDL_Renderer *renderer, int screenWidth, int screenHeight, bool isFullscreen = true);
    void Cleanup();

    // Fullscreen kontrolü
    bool IsFullscreen() const { return m_isFullscreen; }
    void SetFullscreen(bool fullscreen) { m_isFullscreen = fullscreen; }

    // ImGui frame yönetimi
    void BeginFrame();
    void EndFrame(SDL_Renderer *renderer);
    void ProcessEvent(SDL_Event *event);

    // Oyun içi UI (ImGui ile)
    void RenderGameHUD(SDL_Renderer *renderer, int score, int highScore, int snakeLength);
    void RenderBuffIndicator(SDL_Renderer *renderer, int buffType, float duration, int multiplier);

    // Menüler (ImGui ile)
    void RenderMainMenu();
    void RenderSettings();
    void RenderGameOver(int score, int highScore);
    void RenderPauseScreen();

    // Menü yönetimi
    void SetMenuState(MenuState state);
    MenuState GetMenuState() const { return m_currentMenu; }

    // Ayarlar
    int GetMusicVolume() const { return m_musicVolume; }
    int GetSFXVolume() const { return m_sfxVolume; }
    bool IsMusicEnabled() const { return m_musicEnabled; }
    bool IsSFXEnabled() const { return m_sfxEnabled; }
    void SetMusicVolume(int volume);
    void SetSFXVolume(int volume);
    void ToggleMusic();
    void ToggleSFX();

    // Çözünürlük sistemi
    void SetResolutionChangeCallback(std::function<void(int, int)> callback);
    Resolution GetCurrentResolution() const;

    // Callback'ler
    void SetStartGameCallback(std::function<void()> callback) { m_onStartGame = callback; }
    void SetResumeGameCallback(std::function<void()> callback) { m_onResumeGame = callback; }
    void SetReturnToMenuCallback(std::function<void()> callback) { m_onReturnToMenu = callback; }
    void SetQuitGameCallback(std::function<void()> callback) { m_onQuitGame = callback; }

    // Responsive ekran
    void OnResize(int newWidth, int newHeight);
    int GetScreenWidth() const { return m_screenWidth; }
    int GetScreenHeight() const { return m_screenHeight; }
    float GetUIScale() const;

private:
    void InitializeResolutions(int nativeWidth, int nativeHeight);
    void ApplyImGuiStyle();
};
