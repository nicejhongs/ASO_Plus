#pragma once
#include <SDL2/SDL.h>

class Player {
public:
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
    
    // Power-up methods
    void upgradeSpeed();
    void upgradeMissile();
    int getMissileCount() const { return m_missileCount; }
    float getSpeed() const { return m_speed; }

private:
    float m_x, m_y;
    float m_width, m_height;
    float m_speed;
    float m_shootCooldown;
    float m_shootTimer;
    float m_mouseX, m_mouseY;
    int m_missileCount;  // Number of bullets per shot
};
