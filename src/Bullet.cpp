#include "Bullet.h"

Bullet::Bullet(float x, float y, Owner owner, BulletType type)
    : m_x(x)
    , m_y(y)
    , m_width(type == BulletType::MISSILE ? 15.0f : 5.0f)
    , m_height(type == BulletType::MISSILE ? 30.0f : 15.0f)
    , m_speed(type == BulletType::MISSILE ? 800.0f : 500.0f)  // Max speed for missile
    , m_acceleration(type == BulletType::MISSILE ? 1200.0f : 0.0f)  // Missile acceleration
    , m_currentSpeed(type == BulletType::MISSILE ? 100.0f : 500.0f)  // Start slow for missile
    , m_owner(owner)
    , m_type(type)
{
}

Bullet::~Bullet() {
}

void Bullet::update(float deltaTime) {
    // Apply acceleration for missiles
    if (m_type == BulletType::MISSILE && m_owner == Owner::PLAYER) {
        m_currentSpeed += m_acceleration * deltaTime;
        if (m_currentSpeed > m_speed) {
            m_currentSpeed = m_speed;  // Cap at max speed
        }
    }
    
    float moveSpeed = (m_type == BulletType::MISSILE && m_owner == Owner::PLAYER) ? m_currentSpeed : m_speed;
    
    if (m_owner == Owner::PLAYER) {
        // Player bullets move upward
        m_y -= moveSpeed * deltaTime;
    } else {
        // Enemy bullets move downward
        m_y += moveSpeed * deltaTime;
    }
}

void Bullet::render(SDL_Renderer* renderer) {
    if (m_owner == Owner::PLAYER) {
        if (m_type == BulletType::MISSILE) {
            // Missiles are red and thicker
            SDL_SetRenderDrawColor(renderer, 255, 50, 50, 255);
        } else {
            // Lasers are yellow
            SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
        }
    } else {
        // Enemy bullets are orange
        SDL_SetRenderDrawColor(renderer, 255, 150, 0, 255);
    }

    SDL_Rect rect = {
        static_cast<int>(m_x),
        static_cast<int>(m_y),
        static_cast<int>(m_width),
        static_cast<int>(m_height)
    };
    SDL_RenderFillRect(renderer, &rect);
}

bool Bullet::isOffScreen() const {
    return m_y < -20 || m_y > 620; // Remove when it goes off screen top or bottom
}
