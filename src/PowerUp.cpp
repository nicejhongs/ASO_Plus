#include "PowerUp.h"

PowerUp::PowerUp(float x, float y, PowerUpType type)
    : m_x(x)
    , m_y(y)
    , m_width(30.0f)
    , m_height(30.0f)
    , m_speed(80.0f)
    , m_type(type)
    , m_blinkTimer(0.0f)
{
}

PowerUp::~PowerUp() {
}

void PowerUp::update(float deltaTime) {
    m_y += m_speed * deltaTime;
    m_blinkTimer += deltaTime;
}

void PowerUp::render(SDL_Renderer* renderer) {
    // Blinking effect
    if (static_cast<int>(m_blinkTimer * 10) % 2 == 0) {
        SDL_Rect rect = {
            static_cast<int>(m_x),
            static_cast<int>(m_y),
            static_cast<int>(m_width),
            static_cast<int>(m_height)
        };
        
        // Different colors for different types
        switch (m_type) {
            case PowerUpType::SPEED:
                SDL_SetRenderDrawColor(renderer, 0, 255, 255, 255); // Cyan - S
                break;
            case PowerUpType::MISSILE:
                SDL_SetRenderDrawColor(renderer, 255, 165, 0, 255); // Orange - M
                break;
            case PowerUpType::ONE_UP:
                SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255); // Green - U
                break;
        }
        
        SDL_RenderFillRect(renderer, &rect);
        
        // Draw letter
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_Rect letterRect = {
            static_cast<int>(m_x + 8),
            static_cast<int>(m_y + 8),
            14, 14
        };
        SDL_RenderFillRect(renderer, &letterRect);
    }
}

bool PowerUp::isOffScreen() const {
    return m_y > 1200;  // Increased for taller screen
}
