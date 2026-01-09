#pragma once
#include <SDL2/SDL.h>

class Enemy {
public:
    Enemy(float x, float y);
    ~Enemy();

    void update(float deltaTime);
    void render(SDL_Renderer* renderer);
    void render(SDL_Renderer* renderer, SDL_Texture* texture, SDL_Rect* srcRect);

    float getX() const { return m_x; }
    float getY() const { return m_y; }
    float getWidth() const { return m_width; }
    float getHeight() const { return m_height; }

    bool isOffScreen() const;

private:
    float m_x, m_y;
    float m_width, m_height;
    float m_speed;
};
