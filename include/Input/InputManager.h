#pragma once
#include <SDL2/SDL.h>
#include <unordered_map>
#include <functional>

class InputManager
{
private:
    std::unordered_map<SDL_Keycode, bool> m_keyStates;
    std::unordered_map<SDL_Keycode, bool> m_previousKeyStates;

    // Mouse durumları
    std::unordered_map<Uint8, bool> m_mouseButtonStates;
    std::unordered_map<Uint8, bool> m_previousMouseButtonStates;
    int m_mouseX;
    int m_mouseY;

    bool m_quitRequested;

    // Event callback'leri
    std::function<void(int, int)> m_windowResizeCallback;
    std::function<void()> m_mouseUpCallback;
    std::function<void(int, int)> m_mouseMotionCallback;

    InputManager();
    InputManager(const InputManager &) = delete;
    InputManager &operator=(const InputManager &) = delete;

public:
    static InputManager &GetInstance();
    ~InputManager();

    void HandleEvents();
    void HandleEvent(const SDL_Event &event); // Tek event işleme
    void Update();                            // Frame sonu güncelleme

    // Klavye
    bool IsKeyDown(SDL_Keycode key) const;
    bool IsKeyPressed(SDL_Keycode key) const;
    bool IsKeyReleased(SDL_Keycode key) const;

    // Mouse
    bool IsMouseButtonDown(Uint8 button) const;
    bool IsMouseButtonPressed(Uint8 button) const;
    bool IsMouseButtonReleased(Uint8 button) const;
    int GetMouseX() const { return m_mouseX; }
    int GetMouseY() const { return m_mouseY; }

    bool IsQuitRequested() const { return m_quitRequested; }

    void UpdatePreviousStates();

    void Reset();

    // Callback ayarlama
    void SetWindowResizeCallback(std::function<void(int, int)> callback) { m_windowResizeCallback = callback; }
    void SetMouseUpCallback(std::function<void()> callback) { m_mouseUpCallback = callback; }
    void SetMouseMotionCallback(std::function<void(int, int)> callback) { m_mouseMotionCallback = callback; }
};