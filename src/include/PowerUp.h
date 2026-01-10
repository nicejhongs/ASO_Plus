#pragma once
#include <SDL2/SDL.h>
#include <string>

enum class PowerUpType {
    // Main power-ups (need 3 to upgrade)
    SPEED,          // S: Speed up (max 3 stages)
    LASER,          // L: Laser power up (max 3 stages)
    MISSILE,        // M: Missile power up (max 4 stages)
    
    // Energy items
    ENERGY_SMALL,   // E: +1 energy (white)
    ENERGY_MEDIUM,  // E: +4 energy (yellow)
    ENERGY_LARGE,   // E: +8 energy (red)
    
    // Support items
    BONUS,          // B: Bonus points (500-8000)
    KEEP_SPEED,     // K: Keep speed on death (red)
    KEEP_LASER,     // K: Keep laser on death (yellow)
    KEEP_MISSILE,   // K: Keep missile on death (blue)
    ONE_UP,         // P: Extra life
    VOLTAGE,        // V: Increase max energy
    
    // Penalty items (reversed alphabet)
    SPEED_DOWN,     // S̄: Speed down
    LASER_DOWN,     // L̄: Laser down
    MISSILE_DOWN,   // M̄: Missile down
    ENERGY_DOWN,    // Ē: -4 energy
    
    // Armor parts
    ARMOR_HEAD,     // Armor head part
    ARMOR_LEFT,     // Armor left wing
    ARMOR_RIGHT     // Armor right wing
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
    std::string getLabel() const;
    
    bool isOffScreen() const;

private:
    float m_x, m_y;
    float m_width, m_height;
    float m_speed;
    PowerUpType m_type;
    float m_blinkTimer;
};
