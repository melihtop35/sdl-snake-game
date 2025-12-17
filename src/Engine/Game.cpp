#include "../../include/Engine/Game.h"
#include "../../include/Engine/GameManager.h"
#include "../../include/Engine/AudioManager.h"
#include "../../include/Input/InputManager.h"
#include "../../include/UI/UIManager.h"
#include <iostream>
#include <algorithm>

// Yapıcı
Game::Game()
    : m_window(nullptr), m_renderer(nullptr), m_isRunning(false), m_windowWidth(0), m_windowHeight(0), m_windowTitle(""), m_targetFPS(60), m_frameDelay(1000 / 60), m_lastFrameTime(0), m_deltaTime(0.0f), m_gridWidth(0), m_gridHeight(0), m_cellSize(0), m_gridOffsetX(0), m_gridOffsetY(50), m_hudHeight(50), m_basePath("")
{
}

// Yıkıcı
Game::~Game()
{
    Cleanup();
}

// Oyunu başlat
bool Game::Initialize(const std::string &title)
{
    m_windowTitle = title;

    // SDL'i başlat
    if (!InitSDL())
    {
        return false;
    }

    // Tam ekran pencere oluştur
    if (!CreateFullscreenWindow(title))
    {
        return false;
    }

    // Renderer oluştur
    if (!CreateRenderer())
    {
        return false;
    }

    // Grid boyutlarını hesapla
    RecalculateGrid();

    // Audio Manager'ı başlat
    if (!AudioManager::GetInstance().Initialize())
    {
        std::cerr << "AudioManager baslatilamadi!" << std::endl;
        // Ses olmadan da devam edebiliriz
    }

    // UI Manager'ı başlat (ImGui ile)
    if (!UIManager::GetInstance().Initialize(m_renderer, m_windowWidth, m_windowHeight))
    {
        std::cerr << "UIManager baslatilamadi!" << std::endl;
        return false;
    }

    // UI callback'lerini ayarla
    SetupUICallbacks();

    // Game Manager'ı başlat (HUD yüksekliği offset ile)
    GameManager::GetInstance().Initialize(m_gridWidth, m_gridHeight, m_cellSize, m_hudHeight, m_gridOffsetX);

    // Çözünürlük değişikliği callback'i
    UIManager::GetInstance().SetResolutionChangeCallback([this](int newWidth, int newHeight)
                                                         { ChangeResolution(newWidth, newHeight); });

    // Varlıkları yükle
    LoadAssets();

    m_isRunning = true;
    return true;
}

// UI callback'lerini ayarla
void Game::SetupUICallbacks()
{
    UIManager &ui = UIManager::GetInstance();

    // Oyuna başla
    ui.SetStartGameCallback([this]()
                            {
        GameManager::GetInstance().StartGame();
        AudioManager::GetInstance().PlaySoundEffect("gamestart"); });

    // Devam et
    ui.SetResumeGameCallback([]()
                             { GameManager::GetInstance().ResumeGame(); });

    // Ana menüye dön
    ui.SetReturnToMenuCallback([]()
                               {
        GameManager::GetInstance().ReturnToMenu();
        UIManager::GetInstance().SetMenuState(MenuState::MAIN_MENU); });

    // Oyundan çık
    ui.SetQuitGameCallback([this]()
                           { m_isRunning = false; });
}

// SDL başlat
bool Game::InitSDL()
{
    // Ana SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0)
    {
        std::cerr << "SDL baslatilamadi! Hata: " << SDL_GetError() << std::endl;
        return false;
    }

    // Çalıştırılabilir dosyanın bulunduğu dizini al
    char *basePath = SDL_GetBasePath();
    if (basePath)
    {
        m_basePath = basePath;
        SDL_free(basePath);

        // Windows'ta ters slash'ları düzelt
        for (char &c : m_basePath)
        {
            if (c == '\\')
                c = '/';
        }

        std::cout << "Temel yol: " << m_basePath << std::endl;
    }
    else
    {
        m_basePath = "./";
        std::cerr << "SDL_GetBasePath alinamadi, varsayilan kullaniliyor." << std::endl;
    }

    // SDL_image
    int imgFlags = IMG_INIT_PNG | IMG_INIT_JPG;
    if (!(IMG_Init(imgFlags) & imgFlags))
    {
        std::cerr << "SDL_image baslatilamadi! Hata: " << IMG_GetError() << std::endl;
        // Görsel olmadan devam edebiliriz
    }

    return true;
}

// Tam ekran pencere oluştur
bool Game::CreateFullscreenWindow(const std::string &title)
{
    // Ekran boyutlarını al
    SDL_DisplayMode displayMode;
    if (SDL_GetCurrentDisplayMode(0, &displayMode) != 0)
    {
        std::cerr << "Ekran boyutu alinamadi: " << SDL_GetError() << std::endl;
        // Varsayılan değerler kullan
        m_windowWidth = 1920;
        m_windowHeight = 1080;
    }
    else
    {
        m_windowWidth = displayMode.w;
        m_windowHeight = displayMode.h;
    }

    // Tam ekran masaüstü penceresi oluştur
    m_window = SDL_CreateWindow(
        title.c_str(),
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        m_windowWidth,
        m_windowHeight,
        SDL_WINDOW_SHOWN | SDL_WINDOW_FULLSCREEN_DESKTOP);

    if (m_window == nullptr)
    {
        std::cerr << "Pencere olusturulamadi! Hata: " << SDL_GetError() << std::endl;
        return false;
    }

    // Gerçek pencere boyutlarını al
    SDL_GetWindowSize(m_window, &m_windowWidth, &m_windowHeight);

    std::cout << "Tam ekran acildi: " << m_windowWidth << "x" << m_windowHeight << std::endl;

    return true;
}

// Renderer oluştur
bool Game::CreateRenderer()
{
    m_renderer = SDL_CreateRenderer(
        m_window,
        -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    if (m_renderer == nullptr)
    {
        std::cerr << "Renderer olusturulamadi! Hata: " << SDL_GetError() << std::endl;
        return false;
    }

    // Blend mode ayarla (alpha desteği için)
    SDL_SetRenderDrawBlendMode(m_renderer, SDL_BLENDMODE_BLEND);

    return true;
}

// Ana oyun döngüsü
void Game::Run()
{
    m_lastFrameTime = SDL_GetTicks();

    while (m_isRunning)
    {
        // Delta time hesapla
        CalculateDeltaTime();

        // Olayları işle
        HandleEvents();

        // Oyunu güncelle
        Update();

        // Ekrana çiz
        Render();

        // FPS sınırla
        CapFrameRate();
    }
}

// Delta time hesapla
void Game::CalculateDeltaTime()
{
    Uint32 currentTime = SDL_GetTicks();
    m_deltaTime = (currentTime - m_lastFrameTime) / 1000.0f;
    m_lastFrameTime = currentTime;

    // Delta time'ı sınırla (çok büyük atlamalar olmasın)
    if (m_deltaTime > 0.1f)
    {
        m_deltaTime = 0.1f;
    }
}

// FPS sınırlama
void Game::CapFrameRate()
{
    Uint32 frameTime = SDL_GetTicks() - m_lastFrameTime;
    if (frameTime < static_cast<Uint32>(m_frameDelay))
    {
        SDL_Delay(m_frameDelay - frameTime);
    }
}

// Olay işleme
void Game::HandleEvents()
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        // Çıkış kontrolü
        if (event.type == SDL_QUIT)
        {
            m_isRunning = false;
        }

        // PLAYING durumunda klavye event'lerini sadece InputManager'a ilet
        // (ImGui'ye iletme, yoksa ImGui event'i yutar)
        GameState currentState = GameManager::GetInstance().GetCurrentState();

        if (currentState == GameState::PLAYING)
        {
            // Oyun sırasında sadece InputManager klavye event'lerini alsın
            if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP)
            {
                InputManager::GetInstance().HandleEvent(event);

                // ESC tuşu özel - pause için
                if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE)
                {
                    HandleEscapeKey();
                }
            }
            else
            {
                // Mouse ve diğer event'leri ImGui'ye ilet
                UIManager::GetInstance().ProcessEvent(&event);
            }
        }
        else
        {
            // Menülerde ImGui event'leri işlesin
            UIManager::GetInstance().ProcessEvent(&event);

            // ESC tuşu
            if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE)
            {
                HandleEscapeKey();
            }
        }
    }
    // NOT: InputManager::Update() burada DEĞİL, Game::Update'in SONUNDA çağrılmalı!
    // Yoksa IsKeyPressed düzgün çalışmaz.
}

// Güncelleme
void Game::Update()
{
    // Oyun mantığı güncelle
    GameManager::GetInstance().HandleInput();
    GameManager::GetInstance().Update(m_deltaTime);

    // Oyun durumuna göre MenuState'i senkronize et
    GameState currentState = GameManager::GetInstance().GetCurrentState();
    MenuState currentMenuState = UIManager::GetInstance().GetMenuState();

    // GAME_OVER durumuna geçildiyse MenuState'i de güncelle
    if (currentState == GameState::GAME_OVER)
    {
        if (currentMenuState != MenuState::GAME_OVER)
        {
            UIManager::GetInstance().SetMenuState(MenuState::GAME_OVER);
        }
    }
    // PAUSED durumuna geçildiyse ve SETTINGS'de değilse PAUSE_MENU yap
    else if (currentState == GameState::PAUSED)
    {
        if (currentMenuState != MenuState::PAUSE_MENU && currentMenuState != MenuState::SETTINGS)
        {
            UIManager::GetInstance().SetMenuState(MenuState::PAUSE_MENU);
        }
    }

    // Input state'lerini frame sonu için güncelle (IsKeyPressed'in düzgün çalışması için)
    InputManager::GetInstance().Update();
}

// Render
void Game::Render()
{
    // Arka plan rengi
    SDL_SetRenderDrawColor(m_renderer, 30, 30, 30, 255);
    SDL_RenderClear(m_renderer);

    GameState state = GameManager::GetInstance().GetCurrentState();
    MenuState menuState = UIManager::GetInstance().GetMenuState();
    int score = GameManager::GetInstance().GetScore();
    int highScore = GameManager::GetInstance().GetHighScore();

    // ImGui frame başlat
    UIManager::GetInstance().BeginFrame();

    // Oyun durumuna göre render
    switch (state)
    {
    case GameState::MENU:
        if (menuState == MenuState::SETTINGS)
        {
            UIManager::GetInstance().RenderSettings();
        }
        else
        {
            UIManager::GetInstance().RenderMainMenu();
        }
        break;

    case GameState::PLAYING:
    {
        // Oyun alanı
        int gridPixelWidth = m_gridWidth * m_cellSize;
        int gridPixelHeight = m_gridHeight * m_cellSize;

        // Grid çiz
        SDL_SetRenderDrawColor(m_renderer, 40, 40, 40, 255);
        for (int x = 0; x <= m_gridWidth; x++)
        {
            SDL_RenderDrawLine(m_renderer, m_gridOffsetX + x * m_cellSize, m_gridOffsetY,
                               m_gridOffsetX + x * m_cellSize, m_gridOffsetY + gridPixelHeight);
        }
        for (int y = 0; y <= m_gridHeight; y++)
        {
            SDL_RenderDrawLine(m_renderer, m_gridOffsetX, m_gridOffsetY + y * m_cellSize,
                               m_gridOffsetX + gridPixelWidth, m_gridOffsetY + y * m_cellSize);
        }

        // Oyun nesnelerini çiz
        GameManager::GetInstance().Render(m_renderer);

        // HUD (ImGui ile)
        UIManager::GetInstance().RenderGameHUD(m_renderer, score, highScore,
                                               GameManager::GetInstance().GetSnakeLength());

        // Buff göstergesi
        if (GameManager::GetInstance().HasActiveBuff())
        {
            UIManager::GetInstance().RenderBuffIndicator(m_renderer,
                                                         static_cast<int>(GameManager::GetInstance().GetActiveBuffType()),
                                                         GameManager::GetInstance().GetBuffDuration(),
                                                         GameManager::GetInstance().GetScoreMultiplier());
        }
        break;
    }

    case GameState::PAUSED:
    {
        // Oyun alanı
        int gridPixelWidth = m_gridWidth * m_cellSize;
        int gridPixelHeight = m_gridHeight * m_cellSize;

        // Grid çiz
        SDL_SetRenderDrawColor(m_renderer, 40, 40, 40, 255);
        for (int x = 0; x <= m_gridWidth; x++)
        {
            SDL_RenderDrawLine(m_renderer, m_gridOffsetX + x * m_cellSize, m_gridOffsetY,
                               m_gridOffsetX + x * m_cellSize, m_gridOffsetY + gridPixelHeight);
        }
        for (int y = 0; y <= m_gridHeight; y++)
        {
            SDL_RenderDrawLine(m_renderer, m_gridOffsetX, m_gridOffsetY + y * m_cellSize,
                               m_gridOffsetX + gridPixelWidth, m_gridOffsetY + y * m_cellSize);
        }

        // Oyun nesneleri
        GameManager::GetInstance().Render(m_renderer);

        // Pause ekranı veya ayarlar
        if (menuState == MenuState::SETTINGS)
        {
            UIManager::GetInstance().RenderSettings();
        }
        else
        {
            UIManager::GetInstance().RenderPauseScreen();
        }
        break;
    }

    case GameState::GAME_OVER:
    {
        // Oyun alanı
        int gridPixelWidth = m_gridWidth * m_cellSize;
        int gridPixelHeight = m_gridHeight * m_cellSize;

        // Grid çiz
        SDL_SetRenderDrawColor(m_renderer, 40, 40, 40, 255);
        for (int x = 0; x <= m_gridWidth; x++)
        {
            SDL_RenderDrawLine(m_renderer, m_gridOffsetX + x * m_cellSize, m_gridOffsetY,
                               m_gridOffsetX + x * m_cellSize, m_gridOffsetY + gridPixelHeight);
        }
        for (int y = 0; y <= m_gridHeight; y++)
        {
            SDL_RenderDrawLine(m_renderer, m_gridOffsetX, m_gridOffsetY + y * m_cellSize,
                               m_gridOffsetX + gridPixelWidth, m_gridOffsetY + y * m_cellSize);
        }

        // Oyun nesneleri
        GameManager::GetInstance().Render(m_renderer);

        // Game over ekranı
        UIManager::GetInstance().RenderGameOver(score, highScore);
        break;
    }
    }

    // ImGui frame bitir
    UIManager::GetInstance().EndFrame(m_renderer);

    // Ekrana bas
    SDL_RenderPresent(m_renderer);
}

// Varlıkları yükle
bool Game::LoadAssets()
{
    // ============== SES EFEKTLERİ ==============
    AudioManager &audio = AudioManager::GetInstance();
    std::string soundPath = m_basePath + "assets/sounds/";

    // Ses efektlerini yükle
    audio.LoadSoundEffect("eat", soundPath + "eat.wav");
    audio.LoadSoundEffect("gameover", soundPath + "gameover.wav");
    audio.LoadSoundEffect("move", soundPath + "move.wav");
    audio.LoadSoundEffect("gamestart", soundPath + "gamestart.wav");
    audio.LoadSoundEffect("button", soundPath + "button-press.wav");
    audio.LoadSoundEffect("damage", soundPath + "damage.wav");
    audio.LoadSoundEffect("good", soundPath + "good-result.wav");

    // Arka plan müziği
    audio.LoadMusic("background", soundPath + "background.mp3");

    // Arka plan müziğini başlat (varsa)
    if (audio.IsMusicLoaded("background"))
    {
        audio.PlayMusic("background", -1);
        audio.SetMusicVolume(64); // Yarı ses
    }

    // ============== SPRITE'LAR ==============
    std::string spritePath = m_basePath + "assets/sprites/";

    // Cell size'a göre uygun sprite'ları seç (32 veya 64)
    std::string sizeSuffix = (m_cellSize <= 32) ? "_32" : "_64";

    // Yılan sprite'ları (yeşil)
    m_textures["snake_head"] = LoadTexture(spritePath + "snake_green_head" + sizeSuffix + ".png");
    m_textures["snake_body"] = LoadTexture(spritePath + "snake_green_blob" + sizeSuffix + ".png");
    m_textures["snake_tail"] = LoadTexture(spritePath + "snake_green_blob" + sizeSuffix + ".png");
    m_textures["snake_eyes"] = LoadTexture(spritePath + "snake_green_eyes" + sizeSuffix + ".png");

    // Yem sprite'ları
    m_textures["food_apple"] = LoadTexture(spritePath + "apple_red" + sizeSuffix + ".png");
    m_textures["food_oliebol"] = LoadTexture(spritePath + "oliebol" + sizeSuffix + ".png");
    m_textures["food_green"] = LoadTexture(spritePath + "apple_green" + sizeSuffix + ".png");

    // Özel sprite'lar
    m_textures["bomb"] = LoadTexture(spritePath + "bomb" + sizeSuffix + ".png");
    m_textures["easter_egg"] = LoadTexture(spritePath + "easter_egg" + sizeSuffix + ".png");

    // Duvar sprite'ları
    m_textures["wall"] = LoadTexture(spritePath + "wall_block" + sizeSuffix + "_0.png");

    // ============== TEXTURE'LARI OYUN NESNELERİNE ATA ==============
    GameManager &gm = GameManager::GetInstance();

    // Yılan texture'ları
    gm.SetSnakeTextures(
        m_textures["snake_head"],
        m_textures["snake_body"],
        m_textures["snake_tail"]);

    // Tüm yem texture'larını ayarla
    gm.SetFoodTextures(
        m_textures["food_apple"],
        m_textures["food_green"],
        m_textures["food_oliebol"],
        m_textures["easter_egg"],
        m_textures["bomb"]);

    return true;
}

// Texture yükleme yardımcı fonksiyonu
SDL_Texture *Game::LoadTexture(const std::string &path)
{
    SDL_Surface *surface = IMG_Load(path.c_str());
    if (surface == nullptr)
    {
        std::cerr << "Sprite yuklenemedi: " << path << " Hata: " << IMG_GetError() << std::endl;
        return nullptr;
    }

    SDL_Texture *texture = SDL_CreateTextureFromSurface(m_renderer, surface);
    SDL_FreeSurface(surface);

    if (texture == nullptr)
    {
        std::cerr << "Texture olusturulamadi: " << path << " Hata: " << SDL_GetError() << std::endl;
    }

    return texture;
}

// Varlıkları serbest bırak
void Game::UnloadAssets()
{
    // Tüm texture'ları temizle
    for (auto &pair : m_textures)
    {
        if (pair.second != nullptr)
        {
            SDL_DestroyTexture(pair.second);
            pair.second = nullptr;
        }
    }
    m_textures.clear();
}

// Temizleme
void Game::Cleanup()
{
    // Varlıkları temizle
    UnloadAssets();

    // Manager'ları temizle
    UIManager::GetInstance().Cleanup();
    AudioManager::GetInstance().Cleanup();

    // SDL bileşenlerini temizle
    if (m_renderer)
    {
        SDL_DestroyRenderer(m_renderer);
        m_renderer = nullptr;
    }

    if (m_window)
    {
        SDL_DestroyWindow(m_window);
        m_window = nullptr;
    }

    // SDL alt sistemlerini kapat
    IMG_Quit();
    Mix_Quit();
    TTF_Quit();
    SDL_Quit();

    m_isRunning = false;
}

// ESC tuşu işleme
void Game::HandleEscapeKey()
{
    GameState gameState = GameManager::GetInstance().GetCurrentState();
    MenuState menuState = UIManager::GetInstance().GetMenuState();

    if (menuState == MenuState::SETTINGS)
    {
        // Ayarlardan geri dön
        if (gameState == GameState::PAUSED)
        {
            UIManager::GetInstance().SetMenuState(MenuState::PAUSE_MENU);
        }
        else
        {
            UIManager::GetInstance().SetMenuState(MenuState::MAIN_MENU);
        }
    }
    else if (gameState == GameState::MENU)
    {
        // Ana menüden çıkış
        m_isRunning = false;
    }
    else if (gameState == GameState::PLAYING)
    {
        // Oyunu duraklat
        GameManager::GetInstance().PauseGame();
        UIManager::GetInstance().SetMenuState(MenuState::PAUSE_MENU);
    }
    else if (gameState == GameState::PAUSED)
    {
        // Oyuna devam et
        GameManager::GetInstance().ResumeGame();
    }
    else if (gameState == GameState::GAME_OVER)
    {
        // Ana menüye dön
        GameManager::GetInstance().ReturnToMenu();
        UIManager::GetInstance().SetMenuState(MenuState::MAIN_MENU);
    }
}

// Grid boyutunu pencereye göre yeniden hesapla
void Game::RecalculateGrid()
{
    // HUD yüksekliğini çözünürlüğe göre hesapla (1080p baz alınarak)
    // 1080p'de 50px, daha yüksek çözünürlüklerde orantılı artış
    float scale = m_windowHeight / 1080.0f;
    m_hudHeight = static_cast<int>(50.0f * scale);
    m_hudHeight = std::max(40, std::min(100, m_hudHeight)); // 40-100 arası sınırla

    // Kullanılabilir oyun alanı
    int availableWidth = m_windowWidth;
    int availableHeight = m_windowHeight - m_hudHeight;

    // Hücre boyutunu hesapla (en az 20 piksel, en fazla 60 piksel)
    // Grid oranını koru (20:14)
    const int TARGET_GRID_WIDTH = 20;
    const int TARGET_GRID_HEIGHT = 14;

    int cellSizeByWidth = availableWidth / TARGET_GRID_WIDTH;
    int cellSizeByHeight = availableHeight / TARGET_GRID_HEIGHT;

    // Daha küçük olanı kullan (orana uyması için)
    m_cellSize = std::min(cellSizeByWidth, cellSizeByHeight);
    m_cellSize = std::max(20, std::min(60, m_cellSize)); // 20-60 arası sınırla

    // Grid boyutlarını güncelle
    m_gridWidth = availableWidth / m_cellSize;
    m_gridHeight = availableHeight / m_cellSize;

    // Minimum grid boyutu
    m_gridWidth = std::max(10, m_gridWidth);
    m_gridHeight = std::max(8, m_gridHeight);

    // Grid offset'lerini hesapla (ortalama için)
    int gridPixelWidth = m_gridWidth * m_cellSize;
    m_gridOffsetX = (m_windowWidth - gridPixelWidth) / 2;
    m_gridOffsetY = m_hudHeight;

    std::cout << "Grid hesaplandi: " << m_gridWidth << "x" << m_gridHeight << " Cell: " << m_cellSize;
    std::cout << " Offset: (" << m_gridOffsetX << ", " << m_gridOffsetY << ") HUD: " << m_hudHeight << std::endl;
}

// Çözünürlük değiştir
void Game::ChangeResolution(int newWidth, int newHeight)
{
    // Native çözünürlük mü kontrol et
    SDL_DisplayMode displayMode;
    SDL_GetCurrentDisplayMode(0, &displayMode);

    bool isNativeRes = (newWidth == displayMode.w && newHeight == displayMode.h);

    if (isNativeRes)
    {
        // Tam ekran moduna geç
        SDL_SetWindowFullscreen(m_window, SDL_WINDOW_FULLSCREEN_DESKTOP);
        UIManager::GetInstance().SetFullscreen(true);
    }
    else
    {
        // Pencereli moda geç
        SDL_SetWindowFullscreen(m_window, 0);
        SDL_SetWindowSize(m_window, newWidth, newHeight);
        SDL_SetWindowPosition(m_window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
        UIManager::GetInstance().SetFullscreen(false);
    }

    // Gerçek pencere boyutlarını al
    SDL_GetWindowSize(m_window, &m_windowWidth, &m_windowHeight);

    // Grid'i yeniden hesapla
    RecalculateGrid();

    // UI'ı güncelle
    UIManager::GetInstance().OnResize(m_windowWidth, m_windowHeight);

    // GameManager'ı güncelle
    GameManager::GetInstance().OnResize(m_gridWidth, m_gridHeight, m_cellSize, m_gridOffsetX, m_hudHeight);

    std::cout << "Cozunurluk degistirildi: " << m_windowWidth << "x" << m_windowHeight;
    std::cout << (isNativeRes ? " (Tam Ekran)" : " (Pencereli)") << std::endl;
}
