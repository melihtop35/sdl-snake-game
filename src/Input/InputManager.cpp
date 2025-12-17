#include "../../include/Input/InputManager.h"

// Singleton erişim noktası
InputManager &InputManager::GetInstance()
{
    static InputManager instance;
    return instance;
}

// Yapıcı
InputManager::InputManager()
    : m_mouseX(0), m_mouseY(0), m_quitRequested(false),
      m_windowResizeCallback(nullptr), m_mouseUpCallback(nullptr), m_mouseMotionCallback(nullptr)
{
}

// Yıkıcı
InputManager::~InputManager()
{
    // Özel temizlik gerekmiyor
}

// SDL olaylarını işle
void InputManager::HandleEvents()
{
    SDL_Event event;

    // Tüm bekleyen olayları işle
    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
        case SDL_QUIT:
            // Pencere kapatma isteği
            m_quitRequested = true;
            break;

        case SDL_KEYDOWN:
            // Tuş basıldı
            if (!event.key.repeat) // Tekrar basımlarını yoksay
            {
                m_keyStates[event.key.keysym.sym] = true;
            }
            break;

        case SDL_KEYUP:
            // Tuş bırakıldı
            m_keyStates[event.key.keysym.sym] = false;
            break;

        case SDL_MOUSEBUTTONDOWN:
            // Mouse tuşu basıldı
            m_mouseButtonStates[event.button.button] = true;
            m_mouseX = event.button.x;
            m_mouseY = event.button.y;
            break;

        case SDL_MOUSEBUTTONUP:
            // Mouse tuşu bırakıldı
            m_mouseButtonStates[event.button.button] = false;
            // Callback çağır (slider release için)
            if (m_mouseUpCallback)
                m_mouseUpCallback();
            break;

        case SDL_MOUSEMOTION:
            // Mouse hareketi
            m_mouseX = event.motion.x;
            m_mouseY = event.motion.y;
            // Callback çağır (slider drag için)
            if (m_mouseMotionCallback)
                m_mouseMotionCallback(m_mouseX, m_mouseY);
            break;

        case SDL_WINDOWEVENT:
            // Pencere olayları
            if (event.window.event == SDL_WINDOWEVENT_RESIZED ||
                event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED ||
                event.window.event == SDL_WINDOWEVENT_MAXIMIZED ||
                event.window.event == SDL_WINDOWEVENT_RESTORED)
            {
                if (m_windowResizeCallback)
                {
                    // Maximize/restore için pencere boyutunu al
                    SDL_Window *window = SDL_GetWindowFromID(event.window.windowID);
                    if (window)
                    {
                        int w, h;
                        SDL_GetWindowSize(window, &w, &h);
                        m_windowResizeCallback(w, h);
                    }
                }
            }
            break;
        }
    }
}

// Tuş şu anda basılı mı?
bool InputManager::IsKeyDown(SDL_Keycode key) const
{
    auto it = m_keyStates.find(key);
    if (it != m_keyStates.end())
    {
        return it->second;
    }
    return false;
}

// Tuş bu frame'de mi basıldı? (sadece basıldığı an true)
bool InputManager::IsKeyPressed(SDL_Keycode key) const
{
    bool currentState = IsKeyDown(key);

    bool previousState = false;
    auto it = m_previousKeyStates.find(key);
    if (it != m_previousKeyStates.end())
    {
        previousState = it->second;
    }

    // Şu an basılı VE daha önce basılı değildi
    return currentState && !previousState;
}

// Tuş bu frame'de mi bırakıldı?
bool InputManager::IsKeyReleased(SDL_Keycode key) const
{
    bool currentState = IsKeyDown(key);

    bool previousState = false;
    auto it = m_previousKeyStates.find(key);
    if (it != m_previousKeyStates.end())
    {
        previousState = it->second;
    }

    // Şu an basılı değil VE daha önce basılıydı
    return !currentState && previousState;
}

// Frame sonu: mevcut durumları önceki durumlara kopyala
void InputManager::UpdatePreviousStates()
{
    m_previousKeyStates = m_keyStates;
    m_previousMouseButtonStates = m_mouseButtonStates;
}

// Mouse tuşu şu anda basılı mı?
bool InputManager::IsMouseButtonDown(Uint8 button) const
{
    auto it = m_mouseButtonStates.find(button);
    if (it != m_mouseButtonStates.end())
    {
        return it->second;
    }
    return false;
}

// Mouse tuşu bu frame'de mi basıldı?
bool InputManager::IsMouseButtonPressed(Uint8 button) const
{
    bool currentState = IsMouseButtonDown(button);

    bool previousState = false;
    auto it = m_previousMouseButtonStates.find(button);
    if (it != m_previousMouseButtonStates.end())
    {
        previousState = it->second;
    }

    return currentState && !previousState;
}

// Mouse tuşu bu frame'de mi bırakıldı?
bool InputManager::IsMouseButtonReleased(Uint8 button) const
{
    bool currentState = IsMouseButtonDown(button);

    bool previousState = false;
    auto it = m_previousMouseButtonStates.find(button);
    if (it != m_previousMouseButtonStates.end())
    {
        previousState = it->second;
    }

    return !currentState && previousState;
}

// Tüm durumları sıfırla
void InputManager::Reset()
{
    m_keyStates.clear();
    m_previousKeyStates.clear();
    m_mouseButtonStates.clear();
    m_previousMouseButtonStates.clear();
    m_mouseX = 0;
    m_mouseY = 0;
    m_quitRequested = false;
}

// Tek event işleme (Game döngüsünden çağrılır)
void InputManager::HandleEvent(const SDL_Event &event)
{
    switch (event.type)
    {
    case SDL_QUIT:
        m_quitRequested = true;
        break;

    case SDL_KEYDOWN:
        if (!event.key.repeat)
        {
            m_keyStates[event.key.keysym.sym] = true;
        }
        break;

    case SDL_KEYUP:
        m_keyStates[event.key.keysym.sym] = false;
        break;

    case SDL_MOUSEBUTTONDOWN:
        m_mouseButtonStates[event.button.button] = true;
        m_mouseX = event.button.x;
        m_mouseY = event.button.y;
        break;

    case SDL_MOUSEBUTTONUP:
        m_mouseButtonStates[event.button.button] = false;
        break;

    case SDL_MOUSEMOTION:
        m_mouseX = event.motion.x;
        m_mouseY = event.motion.y;
        break;
    }
}

// Frame sonu güncelleme
void InputManager::Update()
{
    UpdatePreviousStates();
}
