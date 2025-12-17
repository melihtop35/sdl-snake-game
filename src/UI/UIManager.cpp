#include "../../include/UI/UIManager.h"
#include "../../include/Engine/AudioManager.h"
#include "../../third_party/imgui/imgui.h"
#include "../../third_party/imgui/imgui_impl_sdl2.h"
#include "../../third_party/imgui/imgui_impl_sdlrenderer2.h"
#include <iostream>
#include <algorithm>

// Singleton erişim noktası
UIManager &UIManager::GetInstance()
{
    static UIManager instance;
    return instance;
}

// Yapıcı
UIManager::UIManager()
    : m_screenWidth(0), m_screenHeight(0), m_nativeWidth(0), m_nativeHeight(0),
      m_isFullscreen(true), m_currentMenu(MenuState::MAIN_MENU),
      m_currentResolutionIndex(0), m_onResolutionChange(nullptr),
      m_musicVolume(50), m_sfxVolume(50), m_musicEnabled(true), m_sfxEnabled(true),
      m_imguiInitialized(false), m_cameFromPause(false),
      m_onStartGame(nullptr), m_onResumeGame(nullptr),
      m_onReturnToMenu(nullptr), m_onQuitGame(nullptr)
{
}

// Yıkıcı
UIManager::~UIManager()
{
    Cleanup();
}

// Başlatma
bool UIManager::Initialize(SDL_Renderer *renderer, int screenWidth, int screenHeight, bool isFullscreen)
{
    m_screenWidth = screenWidth;
    m_screenHeight = screenHeight;
    m_nativeWidth = screenWidth;
    m_nativeHeight = screenHeight;
    m_isFullscreen = isFullscreen;

    // Çözünürlükleri native çözünürlükle başlat
    InitializeResolutions(screenWidth, screenHeight);

    // ImGui context oluştur
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;

    // Klavye navigasyonunu kapat - sadece fare kullanılacak
    io.ConfigFlags &= ~ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags &= ~ImGuiConfigFlags_NavEnableGamepad;

    // ImGui SDL2 ve Renderer backend'lerini başlat
    SDL_Window *window = SDL_RenderGetWindow(renderer);
    if (!ImGui_ImplSDL2_InitForSDLRenderer(window, renderer))
    {
        std::cerr << "ImGui SDL2 backend baslatilamadi!" << std::endl;
        return false;
    }

    if (!ImGui_ImplSDLRenderer2_Init(renderer))
    {
        std::cerr << "ImGui SDL Renderer backend baslatilamadi!" << std::endl;
        return false;
    }

    // Özel stil uygula
    ApplyImGuiStyle();

    m_imguiInitialized = true;
    std::cout << "ImGui basariyla baslatildi!" << std::endl;

    return true;
}

// Temizleme
void UIManager::Cleanup()
{
    if (m_imguiInitialized)
    {
        ImGui_ImplSDLRenderer2_Shutdown();
        ImGui_ImplSDL2_Shutdown();
        ImGui::DestroyContext();
        m_imguiInitialized = false;
    }
}

// ImGui frame başlat
void UIManager::BeginFrame()
{
    if (!m_imguiInitialized)
        return;

    ImGui_ImplSDLRenderer2_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();
}

// ImGui frame bitir ve render et
void UIManager::EndFrame(SDL_Renderer *renderer)
{
    if (!m_imguiInitialized)
        return;

    ImGui::Render();
    ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), renderer);
}

// SDL event'i ImGui'ye ilet
void UIManager::ProcessEvent(SDL_Event *event)
{
    if (!m_imguiInitialized)
        return;

    ImGui_ImplSDL2_ProcessEvent(event);
}

// Özel ImGui stili
void UIManager::ApplyImGuiStyle()
{
    ImGuiStyle &style = ImGui::GetStyle();
    ImVec4 *colors = style.Colors;

    // Koyu oyun teması
    colors[ImGuiCol_WindowBg] = ImVec4(0.1f, 0.1f, 0.15f, 0.95f);
    colors[ImGuiCol_ChildBg] = ImVec4(0.1f, 0.1f, 0.15f, 0.0f);
    colors[ImGuiCol_PopupBg] = ImVec4(0.1f, 0.1f, 0.15f, 0.95f);
    colors[ImGuiCol_Border] = ImVec4(0.3f, 0.3f, 0.4f, 0.5f);
    colors[ImGuiCol_FrameBg] = ImVec4(0.2f, 0.2f, 0.3f, 0.8f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.3f, 0.3f, 0.4f, 0.8f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.4f, 0.4f, 0.5f, 0.8f);
    colors[ImGuiCol_TitleBg] = ImVec4(0.1f, 0.1f, 0.15f, 1.0f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.2f, 0.2f, 0.3f, 1.0f);
    colors[ImGuiCol_MenuBarBg] = ImVec4(0.15f, 0.15f, 0.2f, 1.0f);
    colors[ImGuiCol_Header] = ImVec4(0.3f, 0.5f, 0.3f, 0.8f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.4f, 0.6f, 0.4f, 0.8f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.5f, 0.7f, 0.5f, 0.8f);
    colors[ImGuiCol_Button] = ImVec4(0.3f, 0.5f, 0.3f, 0.8f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.4f, 0.6f, 0.4f, 0.9f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.5f, 0.7f, 0.5f, 1.0f);
    colors[ImGuiCol_SliderGrab] = ImVec4(0.4f, 0.6f, 0.4f, 1.0f);
    colors[ImGuiCol_SliderGrabActive] = ImVec4(0.5f, 0.7f, 0.5f, 1.0f);
    colors[ImGuiCol_Text] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
    colors[ImGuiCol_TextDisabled] = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);

    // Stil ayarları - temel değerler (çözünürlüğe göre ölçeklenecek)
    style.WindowRounding = 8.0f;
    style.FrameRounding = 4.0f;
    style.GrabRounding = 4.0f;
    style.WindowBorderSize = 1.0f;
    style.FrameBorderSize = 0.0f;
    style.WindowPadding = ImVec2(20, 20);
    style.FramePadding = ImVec2(10, 8);
    style.ItemSpacing = ImVec2(10, 10);

    // Font ölçekleme temel değeri
    ImGuiIO &io = ImGui::GetIO();
    io.FontGlobalScale = 1.5f;
}

// Responsive UI için ölçekleme faktörü hesapla (1080p baz alınarak)
float UIManager::GetUIScale() const
{
    // 1080p referans alınarak ölçekleme
    float baseHeight = 1080.0f;
    float scale = m_screenHeight / baseHeight;
    // Minimum ve maksimum sınırlar
    return std::max(0.5f, std::min(scale, 2.5f));
}

// Çözünürlükleri başlat
void UIManager::InitializeResolutions(int nativeWidth, int nativeHeight)
{
    m_resolutions.clear();

    // Standart çözünürlükler (küçükten büyüğe)
    std::vector<std::pair<int, int>> standardRes = {
        {800, 600},
        {1024, 768},
        {1280, 720},
        {1366, 768},
        {1440, 900},
        {1600, 900},
        {1920, 1080},
        {2560, 1440},
        {3200, 2000},
        {3840, 2160}};

    // Sadece native çözünürlükten küçük veya eşit olanları ekle
    for (const auto &res : standardRes)
    {
        if (res.first <= nativeWidth && res.second <= nativeHeight)
        {
            m_resolutions.emplace_back(res.first, res.second);
        }
    }

    // Native çözünürlük listede yoksa ekle
    bool hasNative = false;
    for (const auto &res : m_resolutions)
    {
        if (res.width == nativeWidth && res.height == nativeHeight)
        {
            hasNative = true;
            break;
        }
    }
    if (!hasNative)
    {
        m_resolutions.emplace_back(nativeWidth, nativeHeight);
    }

    // Tam ekran (native) seçeneğini en sona ekle
    Resolution fullscreenRes(nativeWidth, nativeHeight);
    fullscreenRes.label = std::to_string(nativeWidth) + "x" + std::to_string(nativeHeight) + " (Tam Ekran)";

    // Mevcut native çözünürlüğü bul ve label'ını güncelle veya sona ekle
    bool found = false;
    for (size_t i = 0; i < m_resolutions.size(); i++)
    {
        if (m_resolutions[i].width == nativeWidth && m_resolutions[i].height == nativeHeight)
        {
            m_resolutions[i].label = fullscreenRes.label;
            m_currentResolutionIndex = static_cast<int>(i);
            found = true;
            break;
        }
    }

    if (!found)
    {
        m_resolutions.push_back(fullscreenRes);
        m_currentResolutionIndex = static_cast<int>(m_resolutions.size() - 1);
    }

    std::cout << "Cozunurlukler yuklendi. Native: " << nativeWidth << "x" << nativeHeight << std::endl;
    std::cout << "Mevcut cozunurlukler:" << std::endl;
    for (size_t i = 0; i < m_resolutions.size(); i++)
    {
        std::cout << "  " << i << ": " << m_resolutions[i].label << std::endl;
    }
}

// Ana menü render
void UIManager::RenderMainMenu()
{
    if (!m_imguiInitialized)
        return;

    float scale = GetUIScale();

    // Tam ekran pencere
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2((float)m_screenWidth, (float)m_screenHeight));

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                             ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse |
                             ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoScrollbar |
                             ImGuiWindowFlags_NoScrollWithMouse;

    ImGui::Begin("Ana Menu", nullptr, flags);

    // Font ölçekleme
    ImGuiIO &io = ImGui::GetIO();
    io.FontGlobalScale = 1.5f * scale;

    // Başlık
    ImGui::SetCursorPosY(m_screenHeight * 0.15f);
    ImGui::SetWindowFontScale(2.5f);
    float titleWidth = ImGui::CalcTextSize("YILAN OYUNU").x;
    ImGui::SetCursorPosX((m_screenWidth - titleWidth) / 2.0f);
    ImGui::TextColored(ImVec4(0.4f, 0.8f, 0.4f, 1.0f), "YILAN OYUNU");
    ImGui::SetWindowFontScale(1.0f);

    // Butonlar - responsive boyutlar
    float buttonWidth = 250.0f * scale;
    float buttonHeight = 50.0f * scale;
    float centerX = (m_screenWidth - buttonWidth) / 2.0f;
    float startY = m_screenHeight * 0.4f;
    float spacing = 70.0f * scale;

    ImGui::SetCursorPos(ImVec2(centerX, startY));
    if (ImGui::Button("OYUNA BASLA", ImVec2(buttonWidth, buttonHeight)))
    {
        AudioManager::GetInstance().PlaySoundEffect("button");
        if (m_onStartGame)
            m_onStartGame();
    }

    ImGui::SetCursorPos(ImVec2(centerX, startY + spacing));
    if (ImGui::Button("AYARLAR", ImVec2(buttonWidth, buttonHeight)))
    {
        AudioManager::GetInstance().PlaySoundEffect("button");
        m_cameFromPause = false; // Ana menüden gelindi
        m_currentMenu = MenuState::SETTINGS;
    }

    ImGui::SetCursorPos(ImVec2(centerX, startY + spacing * 2));
    if (ImGui::Button("CIKIS", ImVec2(buttonWidth, buttonHeight)))
    {
        AudioManager::GetInstance().PlaySoundEffect("button");
        if (m_onQuitGame)
            m_onQuitGame();
    }

    // Alt bilgi
    ImGui::SetWindowFontScale(0.8f);
    ImGui::SetCursorPos(ImVec2(10 * scale, m_screenHeight - 30 * scale));
    ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "SDL2 + Dear ImGui ile yapildi");
    ImGui::SetWindowFontScale(1.0f);

    // Font ölçeği sıfırla
    io.FontGlobalScale = 1.5f;

    ImGui::End();
}

// Ayarlar menüsü render
void UIManager::RenderSettings()
{
    if (!m_imguiInitialized)
        return;

    float scale = GetUIScale();

    // Tam ekran pencere
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2((float)m_screenWidth, (float)m_screenHeight));

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                             ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse |
                             ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoScrollbar |
                             ImGuiWindowFlags_NoScrollWithMouse;

    ImGui::Begin("Ayarlar", nullptr, flags);

    // Font ölçekleme
    ImGuiIO &io = ImGui::GetIO();
    io.FontGlobalScale = 1.5f * scale;

    // Başlık
    ImGui::SetCursorPosY(m_screenHeight * 0.1f);
    ImGui::SetWindowFontScale(2.0f);
    float titleWidth = ImGui::CalcTextSize("AYARLAR").x;
    ImGui::SetCursorPosX((m_screenWidth - titleWidth) / 2.0f);
    ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "AYARLAR");
    ImGui::SetWindowFontScale(1.0f);

    // Ayarlar paneli - responsive boyutlar
    float panelWidth = 400.0f * scale;
    float panelX = (m_screenWidth - panelWidth) / 2.0f;
    float startY = m_screenHeight * 0.25f;

    ImGui::SetCursorPos(ImVec2(panelX, startY));

    // Çözünürlük dropdown
    if (m_cameFromPause)
    {
        // Oyun sırasında çözünürlük değiştirme kapalı
        ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "Cozunurluk: (oyun sirasinda degistirilemez)");
        ImGui::SetCursorPosX(panelX);
        ImGui::SetNextItemWidth(panelWidth);
        ImGui::BeginDisabled(true);
        ImGui::BeginCombo("##resolution_disabled", m_resolutions[m_currentResolutionIndex].label.c_str());
        ImGui::EndDisabled();
    }
    else
    {
        ImGui::Text("Cozunurluk:");
        ImGui::SetCursorPosX(panelX);
        ImGui::SetNextItemWidth(panelWidth);
        if (ImGui::BeginCombo("##resolution", m_resolutions[m_currentResolutionIndex].label.c_str()))
        {
            for (int i = 0; i < (int)m_resolutions.size(); i++)
            {
                bool isSelected = (m_currentResolutionIndex == i);
                if (ImGui::Selectable(m_resolutions[i].label.c_str(), isSelected))
                {
                    m_currentResolutionIndex = i;
                    AudioManager::GetInstance().PlaySoundEffect("button");
                    if (m_onResolutionChange)
                    {
                        m_onResolutionChange(m_resolutions[i].width, m_resolutions[i].height);
                    }
                }
                if (isSelected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }

        // Zorluk açıklaması
        ImGui::SetCursorPosX(panelX);
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.3f, 1.0f), "* Yuksek cozunurluk = Buyuk oyun haritasi (Zor)");
    }

    ImGui::Spacing();
    ImGui::Spacing();

    // Müzik toggle
    ImGui::SetCursorPosX(panelX);
    if (ImGui::Checkbox("Muzik", &m_musicEnabled))
    {
        AudioManager::GetInstance().PlaySoundEffect("button");
        if (m_musicEnabled)
            AudioManager::GetInstance().ResumeMusic();
        else
            AudioManager::GetInstance().PauseMusic();
    }

    // Müzik ses seviyesi
    ImGui::SetCursorPosX(panelX);
    ImGui::SetNextItemWidth(panelWidth);
    if (ImGui::SliderInt("##musicvol", &m_musicVolume, 0, 100, "Muzik: %d%%"))
    {
        AudioManager::GetInstance().SetMusicVolume(m_musicVolume);
    }

    ImGui::Spacing();

    // Efekt toggle
    ImGui::SetCursorPosX(panelX);
    if (ImGui::Checkbox("Ses Efektleri", &m_sfxEnabled))
    {
        AudioManager::GetInstance().PlaySoundEffect("button");
        AudioManager::GetInstance().SetSFXEnabled(m_sfxEnabled);
    }

    // Efekt ses seviyesi
    ImGui::SetCursorPosX(panelX);
    ImGui::SetNextItemWidth(panelWidth);
    if (ImGui::SliderInt("##sfxvol", &m_sfxVolume, 0, 100, "Efekt: %d%%"))
    {
        AudioManager::GetInstance().SetSFXVolume(m_sfxVolume);
    }

    ImGui::Spacing();
    ImGui::Spacing();
    ImGui::Spacing();

    // Geri butonu - responsive boyut
    float buttonWidth = 200.0f * scale;
    float buttonHeight = 45.0f * scale;
    ImGui::SetCursorPos(ImVec2((m_screenWidth - buttonWidth) / 2.0f, m_screenHeight - 100 * scale));
    if (ImGui::Button("GERI", ImVec2(buttonWidth, buttonHeight)))
    {
        AudioManager::GetInstance().PlaySoundEffect("button");
        // Nereden geldiyse oraya dön
        if (m_cameFromPause)
            m_currentMenu = MenuState::PAUSE_MENU;
        else
            m_currentMenu = MenuState::MAIN_MENU;
    }

    // Font ölçeği sıfırla
    io.FontGlobalScale = 1.5f;

    ImGui::End();
}

// Oyun bitti ekranı
void UIManager::RenderGameOver(int score, int highScore)
{
    if (!m_imguiInitialized)
        return;

    float scale = GetUIScale();

    // Yarı saydam overlay
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2((float)m_screenWidth, (float)m_screenHeight));

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                             ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse |
                             ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoScrollbar |
                             ImGuiWindowFlags_NoScrollWithMouse;

    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.8f));
    ImGui::Begin("Game Over", nullptr, flags);
    ImGui::PopStyleColor();

    // Font ölçekleme
    ImGuiIO &io = ImGui::GetIO();
    io.FontGlobalScale = 1.5f * scale;

    // Başlık
    ImGui::SetCursorPosY(m_screenHeight * 0.2f);
    ImGui::SetWindowFontScale(2.5f);
    float titleWidth = ImGui::CalcTextSize("OYUN BITTI!").x;
    ImGui::SetCursorPosX((m_screenWidth - titleWidth) / 2.0f);
    ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "OYUN BITTI!");
    ImGui::SetWindowFontScale(1.0f);

    // Skor bilgileri
    ImGui::SetCursorPosY(m_screenHeight * 0.4f);
    char scoreText[64];
    snprintf(scoreText, sizeof(scoreText), "Skor: %d", score);
    float scoreWidth = ImGui::CalcTextSize(scoreText).x;
    ImGui::SetCursorPosX((m_screenWidth - scoreWidth) / 2.0f);
    ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "%s", scoreText);

    snprintf(scoreText, sizeof(scoreText), "En Yuksek Skor: %d", highScore);
    scoreWidth = ImGui::CalcTextSize(scoreText).x;
    ImGui::SetCursorPosX((m_screenWidth - scoreWidth) / 2.0f);
    ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "%s", scoreText);

    // Butonlar - responsive
    float buttonWidth = 250.0f * scale;
    float buttonHeight = 50.0f * scale;
    float centerX = (m_screenWidth - buttonWidth) / 2.0f;
    float startY = m_screenHeight * 0.55f;
    float spacing = 70.0f * scale;

    ImGui::SetCursorPos(ImVec2(centerX, startY));
    if (ImGui::Button("TEKRAR OYNA", ImVec2(buttonWidth, buttonHeight)))
    {
        AudioManager::GetInstance().PlaySoundEffect("button");
        if (m_onStartGame)
            m_onStartGame();
    }

    ImGui::SetCursorPos(ImVec2(centerX, startY + spacing));
    if (ImGui::Button("ANA MENU", ImVec2(buttonWidth, buttonHeight)))
    {
        AudioManager::GetInstance().PlaySoundEffect("button");
        if (m_onReturnToMenu)
            m_onReturnToMenu();
    }

    // Font ölçeği sıfırla
    io.FontGlobalScale = 1.5f;

    ImGui::End();
}

// Duraklama ekranı
void UIManager::RenderPauseScreen()
{
    if (!m_imguiInitialized)
        return;

    float scale = GetUIScale();

    // Yarı saydam overlay
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2((float)m_screenWidth, (float)m_screenHeight));

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                             ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse |
                             ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoScrollbar |
                             ImGuiWindowFlags_NoScrollWithMouse;

    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.7f));
    ImGui::Begin("Pause", nullptr, flags);
    ImGui::PopStyleColor();

    // Font ölçekleme
    ImGuiIO &io = ImGui::GetIO();
    io.FontGlobalScale = 1.5f * scale;

    // Başlık
    ImGui::SetCursorPosY(m_screenHeight * 0.25f);
    ImGui::SetWindowFontScale(2.0f);
    float titleWidth = ImGui::CalcTextSize("DURAKLATILDI").x;
    ImGui::SetCursorPosX((m_screenWidth - titleWidth) / 2.0f);
    ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "DURAKLATILDI");
    ImGui::SetWindowFontScale(1.0f);

    // Butonlar - responsive
    float buttonWidth = 250.0f * scale;
    float buttonHeight = 50.0f * scale;
    float centerX = (m_screenWidth - buttonWidth) / 2.0f;
    float startY = m_screenHeight * 0.4f;
    float spacing = 70.0f * scale;

    ImGui::SetCursorPos(ImVec2(centerX, startY));
    if (ImGui::Button("DEVAM ET", ImVec2(buttonWidth, buttonHeight)))
    {
        AudioManager::GetInstance().PlaySoundEffect("button");
        if (m_onResumeGame)
            m_onResumeGame();
    }

    ImGui::SetCursorPos(ImVec2(centerX, startY + spacing));
    if (ImGui::Button("AYARLAR", ImVec2(buttonWidth, buttonHeight)))
    {
        AudioManager::GetInstance().PlaySoundEffect("button");
        m_cameFromPause = true; // Pause'dan geldi - çözünürlük değiştirme kapalı
        m_currentMenu = MenuState::SETTINGS;
    }

    ImGui::SetCursorPos(ImVec2(centerX, startY + spacing * 2));
    if (ImGui::Button("ANA MENU", ImVec2(buttonWidth, buttonHeight)))
    {
        AudioManager::GetInstance().PlaySoundEffect("button");
        if (m_onReturnToMenu)
            m_onReturnToMenu();
    }

    // Font ölçeği sıfırla
    io.FontGlobalScale = 1.5f;

    ImGui::End();
}

// Oyun içi HUD
void UIManager::RenderGameHUD(SDL_Renderer *renderer, int score, int highScore, int snakeLength)
{
    (void)renderer;

    if (!m_imguiInitialized)
        return;

    float scale = GetUIScale();
    float hudHeight = 40.0f * scale;

    // HUD penceresi (üst bar) - responsive
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2((float)m_screenWidth, hudHeight));

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                             ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse |
                             ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoBringToFrontOnFocus;

    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.1f, 0.1f, 0.15f, 0.9f));
    ImGui::Begin("HUD", nullptr, flags);
    ImGui::PopStyleColor();

    // Font ölçekleme - native çözünürlüğe göre
    ImGuiIO &io = ImGui::GetIO();
    float savedFontScale = io.FontGlobalScale;
    io.FontGlobalScale = 1.3f * scale;
    ImGui::SetWindowFontScale(1.0f * scale);

    float padding = 20.0f * scale;
    float yPos = (hudHeight - ImGui::GetFontSize()) / 2.0f;

    // Skor
    ImGui::SetCursorPos(ImVec2(padding, yPos));
    ImGui::Text("Skor: %d", score);

    // Yılan uzunluğu (ortada)
    char lengthText[32];
    snprintf(lengthText, sizeof(lengthText), "Uzunluk: %d", snakeLength);
    float textWidth = ImGui::CalcTextSize(lengthText).x;
    ImGui::SetCursorPos(ImVec2((m_screenWidth - textWidth) / 2.0f, yPos));
    ImGui::Text("%s", lengthText);

    // En yüksek skor (sağda)
    char highScoreText[32];
    snprintf(highScoreText, sizeof(highScoreText), "En Yuksek: %d", highScore);
    textWidth = ImGui::CalcTextSize(highScoreText).x;
    ImGui::SetCursorPos(ImVec2(m_screenWidth - textWidth - padding, yPos));
    ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "%s", highScoreText);

    // Font ölçeği sıfırla
    ImGui::SetWindowFontScale(1.0f);
    io.FontGlobalScale = savedFontScale;

    ImGui::End();
}

// Buff göstergesi
void UIManager::RenderBuffIndicator(SDL_Renderer *renderer, int buffType, float duration, int multiplier)
{
    (void)renderer;

    if (!m_imguiInitialized || buffType == 0)
        return;

    float scale = GetUIScale();
    float buffWidth = 190.0f * scale;
    float buffHeight = 60.0f * scale;

    // Buff penceresi (sağ üst köşe) - responsive
    ImGui::SetNextWindowPos(ImVec2(m_screenWidth - buffWidth - 10 * scale, 50 * scale));
    ImGui::SetNextWindowSize(ImVec2(buffWidth, buffHeight));

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                             ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse |
                             ImGuiWindowFlags_NoScrollbar;

    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.2f, 0.1f, 0.3f, 0.9f));
    ImGui::Begin("Buff", nullptr, flags);
    ImGui::PopStyleColor();

    // Font ölçekleme
    ImGuiIO &io = ImGui::GetIO();
    io.FontGlobalScale = 1.3f * scale;

    if (buffType == 1)
    {
        ImGui::TextColored(ImVec4(0.3f, 0.8f, 1.0f, 1.0f), "HIZ ARTISI");
    }
    else
    {
        ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.3f, 1.0f), "PUAN x%d", multiplier);
    }

    // Kalan süre barı
    ImGui::ProgressBar(duration / 10.0f, ImVec2(-1, 15 * scale), "");

    // Font ölçeği sıfırla
    io.FontGlobalScale = 1.5f;

    ImGui::End();
}

// Menü durumu ayarla
void UIManager::SetMenuState(MenuState state)
{
    m_currentMenu = state;
}

// Müzik ses seviyesi
void UIManager::SetMusicVolume(int volume)
{
    m_musicVolume = std::clamp(volume, 0, 100);
    AudioManager::GetInstance().SetMusicVolume(m_musicVolume);
}

// Efekt ses seviyesi
void UIManager::SetSFXVolume(int volume)
{
    m_sfxVolume = std::clamp(volume, 0, 100);
    AudioManager::GetInstance().SetSFXVolume(m_sfxVolume);
}

// Müzik aç/kapat
void UIManager::ToggleMusic()
{
    m_musicEnabled = !m_musicEnabled;
    if (m_musicEnabled)
        AudioManager::GetInstance().ResumeMusic();
    else
        AudioManager::GetInstance().PauseMusic();
}

// Efekt aç/kapat
void UIManager::ToggleSFX()
{
    m_sfxEnabled = !m_sfxEnabled;
    AudioManager::GetInstance().SetSFXEnabled(m_sfxEnabled);
}

// Çözünürlük callback
void UIManager::SetResolutionChangeCallback(std::function<void(int, int)> callback)
{
    m_onResolutionChange = callback;
}

// Mevcut çözünürlüğü al
Resolution UIManager::GetCurrentResolution() const
{
    if (m_currentResolutionIndex >= 0 && m_currentResolutionIndex < (int)m_resolutions.size())
    {
        return m_resolutions[m_currentResolutionIndex];
    }
    return Resolution(1280, 720);
}

// Ekran boyutu değiştiğinde
void UIManager::OnResize(int newWidth, int newHeight)
{
    m_screenWidth = newWidth;
    m_screenHeight = newHeight;
}
