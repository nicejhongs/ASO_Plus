#include "PowerUp.h"
#include <SDL2/SDL_ttf.h>

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
    // Blinking effect for some items
    bool shouldBlink = (m_type == PowerUpType::SPEED_DOWN || 
                        m_type == PowerUpType::LASER_DOWN || 
                        m_type == PowerUpType::MISSILE_DOWN ||
                        m_type == PowerUpType::ENERGY_DOWN);
    
    if (shouldBlink && static_cast<int>(m_blinkTimer * 4) % 2 == 1) {
        return; // Skip rendering (blink off)
    }
    
    SDL_Rect rect = {
        static_cast<int>(m_x),
        static_cast<int>(m_y),
        static_cast<int>(m_width),
        static_cast<int>(m_height)
    };
    
    // Set color based on type
    switch (m_type) {
        case PowerUpType::SPEED:
            SDL_SetRenderDrawColor(renderer, 0, 255, 255, 255); // Cyan
            break;
        case PowerUpType::LASER:
            SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255); // Yellow
            break;
        case PowerUpType::MISSILE:
            SDL_SetRenderDrawColor(renderer, 255, 165, 0, 255); // Orange
            break;
        case PowerUpType::ENERGY_SMALL:
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); // White
            break;
        case PowerUpType::ENERGY_MEDIUM:
            SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255); // Yellow
            break;
        case PowerUpType::ENERGY_LARGE:
            SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255); // Red
            break;
        case PowerUpType::BONUS:
            SDL_SetRenderDrawColor(renderer, 255, 215, 0, 255); // Gold
            break;
        case PowerUpType::KEEP_SPEED:
            SDL_SetRenderDrawColor(renderer, 255, 100, 100, 255); // Red K
            break;
        case PowerUpType::KEEP_LASER:
            SDL_SetRenderDrawColor(renderer, 255, 255, 100, 255); // Yellow K
            break;
        case PowerUpType::KEEP_MISSILE:
            SDL_SetRenderDrawColor(renderer, 100, 150, 255, 255); // Blue K
            break;
        case PowerUpType::ONE_UP:
            SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255); // Green
            break;
        case PowerUpType::VOLTAGE:
            SDL_SetRenderDrawColor(renderer, 200, 50, 255, 255); // Purple
            break;
        case PowerUpType::SPEED_DOWN:
        case PowerUpType::LASER_DOWN:
        case PowerUpType::MISSILE_DOWN:
        case PowerUpType::ENERGY_DOWN:
            SDL_SetRenderDrawColor(renderer, 139, 69, 19, 255); // Brown (penalty)
            break;
        case PowerUpType::ARMOR_HEAD:
        case PowerUpType::ARMOR_LEFT:
        case PowerUpType::ARMOR_RIGHT:
            SDL_SetRenderDrawColor(renderer, 150, 150, 150, 255); // Gray
            break;
    }
    
    SDL_RenderFillRect(renderer, &rect);
    
    // Draw border
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderDrawRect(renderer, &rect);
}

std::string PowerUp::getLabel() const {
    switch (m_type) {
        case PowerUpType::SPEED: return "S";
        case PowerUpType::LASER: return "L";
        case PowerUpType::MISSILE: return "M";
        case PowerUpType::ENERGY_SMALL:
        case PowerUpType::ENERGY_MEDIUM:
        case PowerUpType::ENERGY_LARGE: return "E";
        case PowerUpType::BONUS: return "B";
        case PowerUpType::KEEP_SPEED:
        case PowerUpType::KEEP_LASER:
        case PowerUpType::KEEP_MISSILE: return "K";
        case PowerUpType::ONE_UP: return "P";
        case PowerUpType::VOLTAGE: return "V";
        case PowerUpType::SPEED_DOWN: return "S";
        case PowerUpType::LASER_DOWN: return "L";
        case PowerUpType::MISSILE_DOWN: return "M";
        case PowerUpType::ENERGY_DOWN: return "E";
        case PowerUpType::ARMOR_HEAD: return "H";
        case PowerUpType::ARMOR_LEFT: return "<";
        case PowerUpType::ARMOR_RIGHT: return ">";
        default: return "?";
    }
}

bool PowerUp::isOffScreen() const {
    return m_y > 1200;  // Increased for taller screen
}
