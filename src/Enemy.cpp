#include "Enemy.h"
#include <cmath>

Enemy::Enemy(float x, float y, EnemyType type, bool isSpecial)
    : m_x(x)
    , m_y(y)
    , m_width(60.0f)
    , m_height(60.0f)
    , m_type(type)
    , m_isSpecial(isSpecial)
    , m_blinkTimer(0.0f)
    , m_movementTimer(0.0f)
    , m_burstCount(0)
{
    // Set parameters based on enemy type
    switch (m_type) {
        case EnemyType::TYPE_01:
            m_speed = 100.0f;  // Slow
            m_horizontalSpeed = 30.0f;  // Slight movement toward player
            m_shootCooldown = 1.5f + (rand() % 1);  // 1.5-2.5 seconds (3x faster)
            break;
        case EnemyType::TYPE_02:
            m_speed = 150.0f;  // Normal
            m_horizontalSpeed = 150.0f;  // Large zigzag (3x)
            m_shootCooldown = 1.0f + (rand() % 1);  // 1-2 seconds (3x faster)
            break;
        case EnemyType::TYPE_03:
            m_speed = 120.0f;  // Slow-normal
            m_horizontalSpeed = 90.0f;  // Larger zigzag (3x)
            m_shootCooldown = 999.0f;  // No shooting (special item enemy)
            break;
        case EnemyType::TYPE_04:
            m_speed = 200.0f;  // Fast
            m_horizontalSpeed = 50.0f;  // Some movement toward player
            m_shootCooldown = 0.7f + (rand() % 1);  // 0.7-1.7 seconds (3x faster)
            break;
        case EnemyType::TYPE_05:
            m_speed = 130.0f;  // Normal
            m_horizontalSpeed = 210.0f;  // Very wide zigzag (3x)
            m_shootCooldown = 1.3f;  // 1.3 seconds between bursts (3x faster)
            m_burstCount = 3;  // 3 bullets per burst
            break;
    }
    
    m_shootTimer = m_shootCooldown;  // Initial delay
}

Enemy::~Enemy() {
}

void Enemy::update(float deltaTime, float playerX) {
    // Move downward
    m_y += m_speed * deltaTime;
    
    // Update movement timer
    m_movementTimer += deltaTime;
    
    // Apply horizontal movement based on type
    switch (m_type) {
        case EnemyType::TYPE_01:
            // Slow movement toward player
            if (playerX >= 0.0f) {
                float directionToPlayer = (playerX > m_x) ? 1.0f : -1.0f;
                m_x += m_horizontalSpeed * directionToPlayer * deltaTime;
            }
            break;
        case EnemyType::TYPE_02:
            // Large zigzag movement
            m_x += m_horizontalSpeed * sin(m_movementTimer * 3.0f) * deltaTime;
            break;
        case EnemyType::TYPE_03:
            // Larger zigzag for special enemy
            m_x += m_horizontalSpeed * sin(m_movementTimer * 2.0f) * deltaTime;
            break;
        case EnemyType::TYPE_04:
            // Fast movement toward player
            if (playerX >= 0.0f) {
                float directionToPlayer = (playerX > m_x) ? 1.0f : -1.0f;
                m_x += m_horizontalSpeed * directionToPlayer * deltaTime;
            }
            break;
        case EnemyType::TYPE_05:
            // Very wide zigzag movement
            m_x += m_horizontalSpeed * sin(m_movementTimer * 2.5f) * deltaTime;
            break;
    }
    
    if (m_isSpecial) {
        m_blinkTimer += deltaTime;
    }
    
    // Update shoot timer
    if (m_shootTimer > 0.0f) {
        m_shootTimer -= deltaTime;
    }
    
    // Reset burst count for TYPE_05 when cooldown finishes
    if (m_type == EnemyType::TYPE_05 && m_shootTimer <= 0.0f && m_burstCount == 0) {
        m_burstCount = 3;
        m_shootCooldown = 1.3f;  // 1.3 seconds between bursts (3x faster)
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
