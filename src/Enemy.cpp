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
    , m_horizontalSpeed(50.0f)  // Horizontal movement speed
    , m_movementTimer(0.0f)
    , m_shootTimer(3.0f + (rand() % 3))  // Random initial delay 3-5 seconds
    , m_shootCooldown(3.0f + (rand() % 3))  // Shoot every 3-5 seconds
{
}

Enemy::~Enemy() {
}

void Enemy::update(float deltaTime) {
    // Move downward
    m_y += m_speed * deltaTime;
    
    // Add horizontal sine wave movement
    m_movementTimer += deltaTime;
    m_x += m_horizontalSpeed * sin(m_movementTimer * 3.0f) * deltaTime;
    
    if (m_isSpecial) {
        m_blinkTimer += deltaTime;
    }
    
    // Update shoot timer
    if (m_shootTimer > 0.0f) {
        m_shootTimer -= deltaTime;
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
        
        // If srcRect is nullptr, use entire texture
        SDL_RenderCopy(renderer, texture, srcRect, &dstRect);
    } else {
        // Fallback: default rendering
        render(renderer);
    }
}
