#pragma once
#include <SDL2/SDL.h>

enum class PowerUpType {
    SPEED,      // S: Speed up
    MISSILE,    // M: Increase bullets
    ONE_UP      // U: Extra life
};

class PowerUp {
public:
    PowerUp(float x, float y, PowerUpType type);
    ~PowerUp();

    void update(float deltaTime);
    void render(SDL_Renderer* renderer);

    float getX() const { return m_x; }
    float getY() const { return m_y; }
    float getWidth() const { return m_width; }
    float getHeight() const { return m_height; }
    PowerUpType getType() const { return m_type; }
    
    bool isOffScreen() const;

private:
    float m_x, m_y;
    float m_width, m_height;
    float m_speed;
    PowerUpType m_type;
    float m_blinkTimer;
};
