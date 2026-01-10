#include "Enemy.h"
#include <cmath>

Enemy::Enemy(float x, float y, bool isSpecial)
    : m_x(x)
    , m_y(y)
    , m_width(60.0f)
    , m_height(60.0f)
    , m_speed(150.0f)
    , m_isSpecial(isSpecial)
    , m_blinkTimer(0.0f)
{
}

Enemy::~Enemy() {
}

void Enemy::update(float deltaTime) {
    // Move downward
    m_y += m_speed * deltaTime;
    
    if (m_isSpecial) {
        m_blinkTimer += deltaTime;
    }
}

void Enemy::render(SDL_Renderer* renderer) {
    // Draw enemy as red rectangle
    SDL_SetRenderDrawColor(renderer, 255, 50, 50, 255);
    SDL_Rect rect = {
        static_cast<int>(m_x),
        static_cast<int>(m_y),
        static_cast<int>(m_width),
        static_cast<int>(m_height)
    };
    SDL_RenderFillRect(renderer, &rect);
}

bool Enemy::isOffScreen() const {
    return m_y > 1200;  // Increased for taller screen
}

void Enemy::render(SDL_Renderer* renderer, SDL_Texture* texture, SDL_Rect* srcRect) {
    if (texture) {
        SDL_Rect dstRect = {
            static_cast<int>(m_x),
            static_cast<int>(m_y),
            static_cast<int>(m_width),
            static_cast<int>(m_height)
        };
        
        // Special enemy blinks
        if (m_isSpecial) {
            Uint8 alpha = static_cast<Uint8>(128 + 127 * sin(m_blinkTimer * 10));
            SDL_SetTextureAlphaMod(texture, alpha);
        }
        
        // If srcRect is nullptr, use entire texture
        SDL_RenderCopy(renderer, texture, srcRect, &dstRect);
        
        // Reset alpha
        if (m_isSpecial) {
            SDL_SetTextureAlphaMod(texture, 255);
        }
    } else {
        // Fallback: default rendering
        render(renderer);
    }
}
