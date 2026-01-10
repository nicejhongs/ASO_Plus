#pragma once
#include <SDL2/SDL.h>

class Enemy {
public:
    Enemy(float x, float y, bool isSpecial = false);
    ~Enemy();

    void update(float deltaTime);
    void render(SDL_Renderer* renderer);
    void render(SDL_Renderer* renderer, SDL_Texture* texture, SDL_Rect* srcRect);

    float getX() const { return m_x; }
    float getY() const { return m_y; }
    float getWidth() const { return m_width; }
    float getHeight() const { return m_height; }
    bool isSpecial() const { return m_isSpecial; }
    
    // Collision hitbox (smaller than sprite)
    float getHitboxX() const { return m_x + m_width * 0.3f; }
    float getHitboxY() const { return m_y + m_height * 0.3f; }
    float getHitboxWidth() const { return m_width * 0.4f; }
    float getHitboxHeight() const { return m_height * 0.4f; }

    bool isOffScreen() const;

private:
    float m_x, m_y;
    float m_width, m_height;
    float m_speed;
    bool m_isSpecial;
    float m_blinkTimer;
};
