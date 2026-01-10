#pragma once
#include <SDL2/SDL.h>

class Bullet {
public:
    enum class Owner {
        PLAYER,
        ENEMY
    };
    
    enum class BulletType {
        LASER,
        MISSILE
    };

    Bullet(float x, float y, Owner owner, BulletType type = BulletType::LASER);
    ~Bullet();

    void update(float deltaTime);
    void render(SDL_Renderer* renderer);

    float getX() const { return m_x; }
    float getY() const { return m_y; }
    float getWidth() const { return m_width; }
    float getHeight() const { return m_height; }
    Owner getOwner() const { return m_owner; }
    BulletType getType() const { return m_type; }

    bool isOffScreen() const;

private:
    float m_x, m_y;
    float m_width, m_height;
    float m_speed;
    Owner m_owner;
    BulletType m_type;
};
