#pragma once
#include <SDL2/SDL.h>

class Enemy {
public:
    enum class EnemyType {
        TYPE_01,  // Straight slow movement, slow shooting (5-7s)
        TYPE_02,  // Zigzag movement, normal shooting (3-5s)
        TYPE_03,  // Special (item drop), no shooting
        TYPE_04,  // Fast straight movement, fast shooting (2-3s)
        TYPE_05   // Circular movement, burst shooting (3 bullets)
    };

    Enemy(float x, float y, EnemyType type, bool isSpecial = false);
    ~Enemy();

    void update(float deltaTime);
    void render(SDL_Renderer* renderer);
    void render(SDL_Renderer* renderer, SDL_Texture* texture, SDL_Rect* srcRect);

    float getX() const { return m_x; }
    float getY() const { return m_y; }
    float getWidth() const { return m_width; }
    float getHeight() const { return m_height; }
    bool isSpecial() const { return m_isSpecial; }
    EnemyType getType() const { return m_type; }
    
    // Collision hitbox (smaller than sprite)
    float getHitboxX() const { return m_x + m_width * 0.3f; }
    float getHitboxY() const { return m_y + m_height * 0.3f; }
    float getHitboxWidth() const { return m_width * 0.4f; }
    float getHitboxHeight() const { return m_height * 0.4f; }

    bool isOffScreen() const;
    
    // Shooting
    bool canShoot() const { return m_shootTimer <= 0.0f; }
    void resetShootTimer() { m_shootTimer = m_shootCooldown; }
    int getBurstCount() const { return m_burstCount; }
    void decreaseBurstCount() { m_burstCount--; }

private:
    float m_x, m_y;
    float m_width, m_height;
    float m_speed;
    EnemyType m_type;
    bool m_isSpecial;
    float m_blinkTimer;
    float m_horizontalSpeed;  // Horizontal movement speed
    float m_movementTimer;    // Timer for sine wave movement
    float m_shootTimer;       // Timer for shooting
    float m_shootCooldown;    // Shoot cooldown (3-5 seconds)
    int m_burstCount;         // Burst shooting counter (for TYPE_05)
};
