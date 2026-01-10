#pragma once
#include <SDL2/SDL.h>

class Player {
public:
    enum class MovementState {
        STOP,
        FORWARD,
        BACKWARD,
        LEFT,
        RIGHT
    };

    Player(float x, float y);
    ~Player();

    void setMousePosition(float mouseX, float mouseY);
    void update(float deltaTime);
    void render(SDL_Renderer* renderer);
    void render(SDL_Renderer* renderer, SDL_Texture* texture, SDL_Rect* srcRect);
    
    void clampToScreen(int screenWidth, int screenHeight);

    float getX() const { return m_x; }
    float getY() const { return m_y; }
    float getWidth() const { return m_width; }
    float getHeight() const { return m_height; }
    
    // Collision hitbox (smaller than sprite)
    float getHitboxX() const { return m_x + m_width * 0.25f; }
    float getHitboxY() const { return m_y + m_height * 0.25f; }
    float getHitboxWidth() const { return m_width * 0.5f; }
    float getHitboxHeight() const { return m_height * 0.5f; }

    bool canShoot() const;
    void resetShootTimer();
    
    // Power-up collection system (need 3 items to upgrade)
    void collectSpeed();
    void collectLaser();
    void collectMissile();
    
    // Direct upgrades/downgrades
    void upgradeSpeed();
    void upgradeLaser();
    void upgradeMissile();
    void downgradeSpeed();
    void downgradeLaser();
    void downgradeMissile();
    
    // Energy system
    void addEnergy(int amount);
    void removeEnergy(int amount);
    void increaseMaxEnergy();
    int getEnergy() const { return m_energy; }
    int getMaxEnergy() const { return m_maxEnergy; }
    bool takeDamage(int amount);  // Returns true if player should explode
    
    // Keep system
    void setKeepSpeed(bool keep) { m_keepSpeed = keep; }
    void setKeepLaser(bool keep) { m_keepLaser = keep; }
    void setKeepMissile(bool keep) { m_keepMissile = keep; }
    
    // Getters for power levels
    int getSpeedLevel() const { return m_speedLevel; }
    int getLaserLevel() const { return m_laserLevel; }
    int getMissileLevel() const { return m_missileLevel; }
    int getSpeedCount() const { return m_speedCount; }
    int getLaserCount() const { return m_laserCount; }
    int getMissileCount() const { return m_missileCount; }
    
    // Reset on death
    void resetOnDeath();
    float getSpeed() const { return m_speed; }
    
    // Movement state
    MovementState getMovementState() const { return m_movementState; }

private:
    float m_x, m_y;
    float m_width, m_height;
    float m_speed;
    float m_shootCooldown;
    float m_shootTimer;
    float m_mouseX, m_mouseY;
    
    // Power-up levels (0-3 for speed/laser, 0-4 for missile)
    int m_speedLevel;
    int m_laserLevel;
    int m_missileLevel;
    
    // Collection counters (need 3 to upgrade)
    int m_speedCount;
    int m_laserCount;
    int m_missileCount;
    
    // Energy system
    int m_energy;
    int m_maxEnergy;
    
    // Keep flags (preserve upgrades on death)
    bool m_keepSpeed;
    bool m_keepLaser;
    bool m_keepMissile;
    
    // Movement state
    MovementState m_movementState;
    float m_lastY;  // Track last Y position
    float m_lastX;  // Track last X position
};
