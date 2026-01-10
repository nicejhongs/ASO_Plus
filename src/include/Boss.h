#pragma once
#include <SDL2/SDL.h>

class Boss {
public:
    enum class BossState {
        ENTERING,   // Boss entering from top
        FIGHTING,   // Active combat
        DYING,      // Death animation
        DEAD        // Fully destroyed
    };

    Boss(float x, float y, int stage);
    ~Boss();

    void update(float deltaTime);
    void render(SDL_Renderer* renderer);
    void render(SDL_Renderer* renderer, SDL_Texture* texture, SDL_Rect* srcRect);

    float getX() const { return m_x; }
    float getY() const { return m_y; }
    float getWidth() const { return m_width; }
    float getHeight() const { return m_height; }
    int getHealth() const { return m_health; }
    int getMaxHealth() const { return m_maxHealth; }
    BossState getState() const { return m_state; }
    
    // Collision hitbox (smaller than sprite)
    float getHitboxX() const { return m_x + m_width * 0.2f; }
    float getHitboxY() const { return m_y + m_height * 0.2f; }
    float getHitboxWidth() const { return m_width * 0.6f; }
    float getHitboxHeight() const { return m_height * 0.6f; }

    void takeDamage(int damage);
    bool isOffScreen() const;
    
    // Shooting
    bool canShoot() const { return m_shootTimer <= 0.0f; }
    void resetShootTimer() { m_shootTimer = m_shootCooldown; }
    
    // Get cannon positions for shooting (12 cannons)
    struct CannonPos {
        float offsetX;
        float offsetY;
    };
    const CannonPos* getCannonPositions() const { return m_cannonPositions; }
    int getCannonCount() const { return 12; }
    
    // Weak point (front center) - takes 5x damage
    float getWeakPointX() const { return m_x + m_width * 0.5f - 8.0f; }
    float getWeakPointY() const { return m_y + m_height * 0.5f - 8.0f; }
    float getWeakPointWidth() const { return 16.0f; }
    float getWeakPointHeight() const { return 16.0f; }

private:
    float m_x, m_y;
    float m_width, m_height;
    float m_speed;
    int m_stage;
    int m_health;
    int m_maxHealth;
    BossState m_state;
    
    float m_movementTimer;
    float m_shootTimer;
    float m_shootCooldown;
    float m_targetY;  // Target Y position for entering
    
    float m_deathTimer;  // Death animation timer
    
    // 12 cannon positions (relative to boss center)
    CannonPos m_cannonPositions[12];
};
