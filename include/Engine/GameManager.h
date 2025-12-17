#pragma once
#include <SDL2/SDL.h>
#include <memory>
#include "../Game/Snake.h"
#include "../Game/Food.h"
#include "../Game/FoodPool.h"

// Oyun durumları
enum class GameState
{
    MENU,
    PLAYING,
    PAUSED,
    GAME_OVER
};

// Buff türleri (Easter Egg)
enum class BuffType
{
    NONE,
    SPEED_BOOST,     // Hız artışı
    SCORE_MULTIPLIER // Puan çarpanı
};

class GameManager
{
private:
    GameState m_currentState;
    int m_score;
    int m_highScore;
    std::unique_ptr<Snake> m_snake;
    std::unique_ptr<FoodPool> m_foodPool; // Food yerine FoodPool
    int m_gridWidth;
    int m_gridHeight;
    int m_cellSize;
    int m_hudOffset;   // HUD için dikey offset (piksel)
    int m_gridOffsetX; // Grid için yatay offset (ortalama)

    // Buff sistemi
    BuffType m_activeBuffType;
    float m_buffDuration;      // Kalan buff süresi
    int m_scoreMultiplier;     // Puan çarpanı (1, 2, 3, veya 5)
    float m_originalMoveDelay; // Orijinal yılan hızı

    GameManager();
    GameManager(const GameManager &) = delete;
    GameManager &operator=(const GameManager &) = delete;

public:
    static GameManager &GetInstance();
    ~GameManager();

    void Initialize(int gridWidth, int gridHeight, int cellSize, int hudOffset = 50, int gridOffsetX = 0);
    void Update(float deltaTime);
    void HandleInput();
    void Render(SDL_Renderer *renderer);

    // Oyun durumu
    void ResetGame();
    void PauseGame();
    void ResumeGame();
    void StartGame();
    void ReturnToMenu();

    // Durum yönetimi
    GameState GetCurrentState() const { return m_currentState; }
    void SetCurrentState(GameState state) { m_currentState = state; }

    // Skor yönetimi
    int GetScore() const { return m_score; }
    int GetHighScore() const { return m_highScore; }
    void AddScore(int points);

    // Yılan bilgisi
    int GetSnakeLength() const;

    Snake *GetSnake() const { return m_snake.get(); }
    FoodPool *GetFoodPool() const { return m_foodPool.get(); }

    // Texture ayarlama
    void SetSnakeTextures(SDL_Texture *head, SDL_Texture *body, SDL_Texture *tail);
    void SetFoodTextures(SDL_Texture *appleRed, SDL_Texture *appleGreen,
                         SDL_Texture *oliebol, SDL_Texture *easterEgg, SDL_Texture *bomb);

    // Buff sistemi
    BuffType GetActiveBuffType() const { return m_activeBuffType; }
    int GetScoreMultiplier() const { return m_scoreMultiplier; }
    float GetBuffDuration() const { return m_buffDuration; }
    bool HasActiveBuff() const { return m_activeBuffType != BuffType::NONE; }

    // Responsive ekran
    void OnResize(int newGridWidth, int newGridHeight, int newCellSize, int newGridOffsetX, int newHudOffset);
    int GetGridWidth() const { return m_gridWidth; }
    int GetGridHeight() const { return m_gridHeight; }
    int GetCellSize() const { return m_cellSize; }

private:
    void CheckCollisions();
    void CheckFoodCollision();
    void CheckWallCollision();
    void CheckSelfCollision();

    // Yılanın pozisyonlarını yasak liste olarak al
    std::vector<std::pair<int, int>> GetSnakePositions() const;

    // Buff yönetimi
    void ApplyRandomBuff();
    void UpdateBuff(float deltaTime);
    void ClearBuff();

    // Yılan ölüm efekti tetikleme
    void TriggerSnakeDeathEffect();
};