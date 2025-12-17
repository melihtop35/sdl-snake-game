#include "../../include/Engine/GameManager.h"
#include "../../include/Input/InputManager.h"
#include "../../include/Engine/AudioManager.h"
#include "../../include/Effects/ParticleSystem.h"
#include <iostream>
#include <random>
#include <chrono>

// Singleton erişim noktası
GameManager &GameManager::GetInstance()
{
    static GameManager instance;
    return instance;
}

// Yapıcı
GameManager::GameManager()
    : m_currentState(GameState::MENU), m_score(0), m_highScore(0), m_snake(nullptr), m_foodPool(nullptr), m_gridWidth(0), m_gridHeight(0), m_cellSize(0), m_hudOffset(50), m_gridOffsetX(0), m_activeBuffType(BuffType::NONE), m_buffDuration(0.0f), m_scoreMultiplier(1), m_originalMoveDelay(0.15f)
{
}

// Yıkıcı
GameManager::~GameManager()
{
    // unique_ptr otomatik temizler
}

// Oyunu başlat
void GameManager::Initialize(int gridWidth, int gridHeight, int cellSize, int hudOffset, int gridOffsetX)
{
    m_gridWidth = gridWidth;
    m_gridHeight = gridHeight;
    m_cellSize = cellSize;
    m_hudOffset = hudOffset;
    m_gridOffsetX = gridOffsetX;

    // Yılanın başlangıç pozisyonu (grid ortası, offset'ler dahil)
    int startX = (gridWidth / 2) * cellSize + m_gridOffsetX;
    int startY = (gridHeight / 2) * cellSize + m_hudOffset;

    // Yılanı oluştur
    m_snake = std::make_unique<Snake>(startX, startY, cellSize);
    m_snake->SetMoveDelay(0.15f); // 150ms hareket gecikmesi
    m_originalMoveDelay = 0.15f;

    // FoodPool oluştur
    m_foodPool = std::make_unique<FoodPool>();
    m_foodPool->Initialize(gridWidth, gridHeight, cellSize, m_hudOffset, m_gridOffsetX,
                           15,                // pool size
                           10);               // max active foods
    m_foodPool->SetSpawnInterval(2.0f, 4.0f); // 2-4 saniye arasında spawn

    // Buff'ı sıfırla
    ClearBuff();

    // Başlangıç durumu
    m_currentState = GameState::MENU;
    m_score = 0;
}

// Her frame çağrılır
void GameManager::Update(float deltaTime)
{
    // Sadece oyun devam ediyorken güncelle
    if (m_currentState == GameState::PLAYING)
    {
        m_snake->Update(deltaTime);

        // FoodPool güncelle (spawn zamanlayıcı dahil)
        auto forbiddenPositions = GetSnakePositions();
        m_foodPool->Update(deltaTime, forbiddenPositions);

        // Buff zamanlayıcısını güncelle
        UpdateBuff(deltaTime);

        // Çarpışma kontrolleri
        CheckCollisions();
    }
}

// Girdi işleme
void GameManager::HandleInput()
{
    InputManager &input = InputManager::GetInstance();

    switch (m_currentState)
    {
    case GameState::MENU:
        // Enter ile oyunu başlat
        if (input.IsKeyPressed(SDLK_RETURN) || input.IsKeyPressed(SDLK_KP_ENTER))
        {
            AudioManager::GetInstance().PlaySoundEffect("gamestart");
            ResetGame();
            m_currentState = GameState::PLAYING;
        }
        // Escape ile çık
        if (input.IsKeyPressed(SDLK_ESCAPE))
        {
            // Game sınıfı bu durumu kontrol edecek
        }
        break;

    case GameState::PLAYING:
        // Yön kontrolleri
        if (input.IsKeyPressed(SDLK_UP) || input.IsKeyPressed(SDLK_w))
        {
            m_snake->SetDirection(Direction::UP);
            AudioManager::GetInstance().PlaySoundEffect("move");
        }
        else if (input.IsKeyPressed(SDLK_DOWN) || input.IsKeyPressed(SDLK_s))
        {
            m_snake->SetDirection(Direction::DOWN);
            AudioManager::GetInstance().PlaySoundEffect("move");
        }
        else if (input.IsKeyPressed(SDLK_LEFT) || input.IsKeyPressed(SDLK_a))
        {
            m_snake->SetDirection(Direction::LEFT);
            AudioManager::GetInstance().PlaySoundEffect("move");
        }
        else if (input.IsKeyPressed(SDLK_RIGHT) || input.IsKeyPressed(SDLK_d))
        {
            m_snake->SetDirection(Direction::RIGHT);
            AudioManager::GetInstance().PlaySoundEffect("move");
        }

        // Duraklat
        if (input.IsKeyPressed(SDLK_p) || input.IsKeyPressed(SDLK_ESCAPE))
        {
            AudioManager::GetInstance().PlaySoundEffect("button");
            PauseGame();
        }
        break;

    case GameState::PAUSED:
        // Devam et
        if (input.IsKeyPressed(SDLK_p) || input.IsKeyPressed(SDLK_RETURN))
        {
            AudioManager::GetInstance().PlaySoundEffect("button");
            ResumeGame();
        }
        // Menüye dön
        if (input.IsKeyPressed(SDLK_ESCAPE))
        {
            AudioManager::GetInstance().PlaySoundEffect("button");
            m_currentState = GameState::MENU;
        }
        break;

    case GameState::GAME_OVER:
        // Tekrar oyna
        if (input.IsKeyPressed(SDLK_RETURN) || input.IsKeyPressed(SDLK_KP_ENTER))
        {
            AudioManager::GetInstance().PlaySoundEffect("gamestart");
            ResetGame();
            m_currentState = GameState::PLAYING;
        }
        // Menüye dön
        if (input.IsKeyPressed(SDLK_ESCAPE))
        {
            AudioManager::GetInstance().PlaySoundEffect("button");
            m_currentState = GameState::MENU;
        }
        break;
    }
}

// Render
void GameManager::Render(SDL_Renderer *renderer)
{
    // Oyun nesnelerini çiz (PLAYING veya PAUSED durumunda)
    if (m_currentState == GameState::PLAYING || m_currentState == GameState::PAUSED)
    {
        if (m_foodPool)
        {
            m_foodPool->Render(renderer);
        }
        if (m_snake)
        {
            m_snake->Render(renderer);
        }
    }
}

// Oyunu sıfırla
void GameManager::ResetGame()
{
    m_score = 0;

    // Yılanı başlangıç pozisyonuna sıfırla (offset'ler dahil)
    int startX = (m_gridWidth / 2) * m_cellSize + m_gridOffsetX;
    int startY = (m_gridHeight / 2) * m_cellSize + m_hudOffset;

    if (m_snake)
    {
        m_snake->Reset(startX, startY);
        m_snake->SetMoveDelay(m_originalMoveDelay);
    }

    // Buff'ı temizle
    ClearBuff();

    // Tüm food'ları deaktif et ve yeniden başlat
    if (m_foodPool)
    {
        m_foodPool->DeactivateAll();
        // İlk food'u hemen spawn et
        auto forbiddenPositions = GetSnakePositions();
        m_foodPool->SpawnFood(forbiddenPositions);
    }

    m_currentState = GameState::PLAYING;
}

// Duraklat
void GameManager::PauseGame()
{
    if (m_currentState == GameState::PLAYING)
    {
        m_currentState = GameState::PAUSED;
        AudioManager::GetInstance().PauseMusic();
    }
}

// Devam et
void GameManager::ResumeGame()
{
    if (m_currentState == GameState::PAUSED)
    {
        m_currentState = GameState::PLAYING;
        AudioManager::GetInstance().ResumeMusic();
    }
}

// Oyunu başlat
void GameManager::StartGame()
{
    ResetGame();
    m_currentState = GameState::PLAYING;
}

// Menüye dön
void GameManager::ReturnToMenu()
{
    m_currentState = GameState::MENU;
    m_score = 0;

    // Yılanı sıfırla (offset'ler dahil)
    int startX = (m_gridWidth / 2) * m_cellSize + m_gridOffsetX;
    int startY = (m_gridHeight / 2) * m_cellSize + m_hudOffset;
    if (m_snake)
    {
        m_snake->Reset(startX, startY);
    }
}

// Yılan uzunluğu
int GameManager::GetSnakeLength() const
{
    if (m_snake)
    {
        return static_cast<int>(m_snake->GetBody().size());
    }
    return 0;
}

// Skor ekle
void GameManager::AddScore(int points)
{
    m_score += points;

    // Skor negatife düşmesin
    if (m_score < 0)
        m_score = 0;

    // Yeni yüksek skor mu?
    if (m_score > m_highScore)
    {
        m_highScore = m_score;
        if (m_highScore % 100 == 0)
            AudioManager::GetInstance().PlaySoundEffect("good");
    }
}

// Tüm çarpışma kontrolleri
void GameManager::CheckCollisions()
{
    CheckFoodCollision();
    CheckWallCollision();
    CheckSelfCollision();
}

// Yem çarpışması
void GameManager::CheckFoodCollision()
{
    if (!m_snake || !m_foodPool)
        return;

    SnakeSegment head = m_snake->GetHeadPosition();

    // Yılan kafasının pozisyonundaki food'u bul
    Food *collidedFood = m_foodPool->GetFoodAt(head.x, head.y);

    if (!collidedFood)
        return;

    FoodType foodType = collidedFood->GetType();

    // Food pozisyonunu al (parçacık efekti için)
    int foodX, foodY;
    collidedFood->GetPosition(foodX, foodY);

    // Parçacık efekti oynat (tüm yem türleri için)
    ParticleSystem::GetInstance().PlayFoodCollect(
        static_cast<float>(foodX),
        static_cast<float>(foodY),
        static_cast<int>(foodType));

    switch (foodType)
    {
    case FoodType::APPLE_RED:
    case FoodType::APPLE_GREEN:
    case FoodType::OLIEBOL:
        // Normal yemler: puan ekle ve büyü
        AddScore(collidedFood->GetPointValue() * m_scoreMultiplier);
        m_snake->Grow();
        AudioManager::GetInstance().PlaySoundEffect("eat");
        break;

    case FoodType::EASTER_EGG:
        // Sürpriz yumurta: puan + rastgele buff
        AddScore(collidedFood->GetPointValue() * m_scoreMultiplier);
        m_snake->Grow();
        ApplyRandomBuff();
        AudioManager::GetInstance().PlaySoundEffect("good");
        break;

    case FoodType::BOMB:
    {
        // Bomba: 1 parça kaybet ve -20 puan
        AddScore(-20);

        // Shrink olmadan önce kuyruk pozisyonunu al (patlama efekti için)
        const auto &body = m_snake->GetBody();
        if (body.size() > 1)
        {
            const SnakeSegment &tail = body.back();
            ParticleSystem::GetInstance().PlayBombDamage(
                static_cast<float>(tail.x),
                static_cast<float>(tail.y));
        }

        m_snake->Shrink(1);
        AudioManager::GetInstance().PlaySoundEffect("damage");

        // Eğer yılan çok küçüldüyse oyun biter
        if (m_snake->GetLength() <= 1)
        {
            m_currentState = GameState::GAME_OVER;
            AudioManager::GetInstance().PlaySoundEffect("gameover");
            return;
        }
        break;
    }
    }

    // Yenen food'u deaktif et (pool'a geri dön)
    m_foodPool->DeactivateFood(collidedFood);
}

// Duvar çarpışması
void GameManager::CheckWallCollision()
{
    if (!m_snake)
        return;

    SnakeSegment head = m_snake->GetHeadPosition();
    int minX = m_gridOffsetX;
    int maxX = m_gridWidth * m_cellSize + m_gridOffsetX;
    int minY = m_hudOffset;
    int maxY = m_gridHeight * m_cellSize + m_hudOffset;

    if (head.x < minX || head.x >= maxX || head.y < minY || head.y >= maxY)
    {
        // Duvara çarptı! Yılan ölüm sekansı başlat
        TriggerSnakeDeathEffect();
        m_currentState = GameState::GAME_OVER;
        AudioManager::GetInstance().PlaySoundEffect("damage");
        AudioManager::GetInstance().PlaySoundEffect("gameover");
    }
}

// Kendine çarpma
void GameManager::CheckSelfCollision()
{
    if (!m_snake)
        return;

    if (m_snake->CheckSelfCollision())
    {
        // Kendine çarptı! Yılan ölüm sekansı başlat
        TriggerSnakeDeathEffect();
        m_currentState = GameState::GAME_OVER;
        AudioManager::GetInstance().PlaySoundEffect("damage");
        AudioManager::GetInstance().PlaySoundEffect("gameover");
    }
}

// Yılanın pozisyonlarını yasak liste olarak al
std::vector<std::pair<int, int>> GameManager::GetSnakePositions() const
{
    std::vector<std::pair<int, int>> positions;

    if (m_snake)
    {
        const auto &body = m_snake->GetBody();
        for (const auto &segment : body)
        {
            positions.push_back({segment.x, segment.y});
        }
    }

    return positions;
}

// Yılan texture'larını ayarla
void GameManager::SetSnakeTextures(SDL_Texture *head, SDL_Texture *body, SDL_Texture *tail)
{
    if (m_snake)
    {
        m_snake->SetTextures(head, body, tail);
    }
}

// Tüm yem texture'larını ayarla
void GameManager::SetFoodTextures(SDL_Texture *appleRed, SDL_Texture *appleGreen,
                                  SDL_Texture *oliebol, SDL_Texture *easterEgg, SDL_Texture *bomb)
{
    if (m_foodPool)
    {
        m_foodPool->SetFoodTextures(appleRed, appleGreen, oliebol, easterEgg, bomb);
    }
}

// Rastgele buff uygula (Easter Egg)
void GameManager::ApplyRandomBuff()
{
    // Rastgele sayı üreteci
    static std::mt19937 rng(static_cast<unsigned int>(
        std::chrono::high_resolution_clock::now().time_since_epoch().count()));

    // Buff türü seç (50% hız, 50% puan çarpanı)
    std::uniform_int_distribution<int> buffDist(0, 1);
    int buffChoice = buffDist(rng);

    // Buff süresi (5-10 saniye)
    std::uniform_real_distribution<float> durationDist(5.0f, 10.0f);
    m_buffDuration = durationDist(rng);

    if (buffChoice == 0)
    {
        // Hız buffu
        m_activeBuffType = BuffType::SPEED_BOOST;
        if (m_snake)
        {
            // %30 daha hızlı
            m_snake->SetMoveDelay(m_originalMoveDelay * 0.7f);
        }
    }
    else
    {
        // Puan çarpanı buffu
        m_activeBuffType = BuffType::SCORE_MULTIPLIER;

        // Rastgele çarpan seç (2x, 3x veya 5x)
        std::uniform_int_distribution<int> multDist(0, 2);
        int multChoice = multDist(rng);
        switch (multChoice)
        {
        case 0:
            m_scoreMultiplier = 2;
            break;
        case 1:
            m_scoreMultiplier = 3;
            break;
        case 2:
            m_scoreMultiplier = 5;
            break;
        }
    }
}

// Buff zamanlayıcısını güncelle
void GameManager::UpdateBuff(float deltaTime)
{
    if (m_activeBuffType == BuffType::NONE)
        return;

    m_buffDuration -= deltaTime;

    if (m_buffDuration <= 0)
    {
        ClearBuff();
    }
}

// Buff'ı temizle
void GameManager::ClearBuff()
{
    m_activeBuffType = BuffType::NONE;
    m_buffDuration = 0.0f;
    m_scoreMultiplier = 1;

    // Hızı normale döndür
    if (m_snake)
    {
        m_snake->SetMoveDelay(m_originalMoveDelay);
    }
}

// Yılan ölüm efekti tetikleme
void GameManager::TriggerSnakeDeathEffect()
{
    if (!m_snake)
        return;

    const auto &body = m_snake->GetBody();
    std::vector<std::pair<float, float>> segments;

    // Baştan kuyruğa tüm segment pozisyonlarını al
    for (const auto &segment : body)
    {
        segments.push_back({static_cast<float>(segment.x),
                            static_cast<float>(segment.y)});
    }

    // Sıralı patlama efektini başlat (her segment 0.05 saniye arayla patlayacak)
    ParticleSystem::GetInstance().PlaySnakeDeathSequence(segments, 0.05f);
}

// Pencere yeniden boyutlandırıldığında
void GameManager::OnResize(int newGridWidth, int newGridHeight, int newCellSize, int newGridOffsetX, int newHudOffset)
{
    m_gridWidth = newGridWidth;
    m_gridHeight = newGridHeight;
    m_cellSize = newCellSize;
    m_gridOffsetX = newGridOffsetX;
    m_hudOffset = newHudOffset;

    // Yılanın pozisyonunu sınırlar içinde tut
    if (m_snake)
    {
        // Yılanın her parçasını kontrol et ve gerekirse sınırla
        // Basitçe yılanı ortada yeniden başlatıyoruz
        // (alternatif: sadece sınır dışındaki parçaları kırp)
    }

    // FoodPool'u yeni boyutlarla güncelle
    if (m_foodPool)
    {
        m_foodPool->OnResize(newGridWidth, newGridHeight, newCellSize, newGridOffsetX, newHudOffset);
    }
}