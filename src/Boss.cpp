#include "Boss.h"
#include <cmath>

Boss::Boss(float x, float y, int stage)
    : m_x(x)
    , m_y(y)
    , m_width(240.0f)
    , m_height(240.0f)
    , m_speed(100.0f)
    , m_stage(stage)
    , m_health(100)
    , m_maxHealth(100)
    , m_state(BossState::ENTERING)
    , m_movementTimer(0.0f)
    , m_shootTimer(2.0f)
    , m_shootCooldown(2.0f)
    , m_targetY(100.0f)
    , m_deathTimer(0.0f)
{
    // Initialize 12 cannon positions (based on boss01.png image analysis)
    // Boss is 240x240 (2x scale), positions are relative to boss top-left corner
    // Format: {offsetX, offsetY} from boss top-left
    
    // Top row (4 cannons)
    m_cannonPositions[0] = {40.0f, 30.0f};    // Top-left
    m_cannonPositions[1] = {80.0f, 20.0f};    // Top-left-center
    m_cannonPositions[2] = {140.0f, 20.0f};   // Top-right-center
    m_cannonPositions[3] = {190.0f, 30.0f};   // Top-right
    
    // Middle row (4 cannons)
    m_cannonPositions[4] = {20.0f, 100.0f};   // Middle-left
    m_cannonPositions[5] = {70.0f, 90.0f};    // Middle-left-center
    m_cannonPositions[6] = {150.0f, 90.0f};   // Middle-right-center
    m_cannonPositions[7] = {210.0f, 100.0f};  // Middle-right
    
    // Bottom row (4 cannons)
    m_cannonPositions[8] = {40.0f, 190.0f};   // Bottom-left
    m_cannonPositions[9] = {80.0f, 200.0f};   // Bottom-left-center
    m_cannonPositions[10] = {140.0f, 200.0f}; // Bottom-right-center
    m_cannonPositions[11] = {190.0f, 190.0f}; // Bottom-right
}

Boss::~Boss() {
}

void Boss::update(float deltaTime) {
    m_movementTimer += deltaTime;
    
    switch (m_state) {
        case BossState::ENTERING:
            // Move down to target position
            if (m_y < m_targetY) {
                m_y += m_speed * deltaTime;
            } else {
                m_y = m_targetY;
                m_state = BossState::FIGHTING;
            }
            break;
            
        case BossState::FIGHTING:
            // Horizontal movement pattern (sine wave)
            m_x += 80.0f * sin(m_movementTimer * 1.5f) * deltaTime;
            
            // Update shoot timer
            if (m_shootTimer > 0.0f) {
                m_shootTimer -= deltaTime;
            }
            break;
            
        case BossState::DYING:
            m_deathTimer += deltaTime;
            if (m_deathTimer >= 2.0f) {
                m_state = BossState::DEAD;
            }
            break;
            
        case BossState::DEAD:
            // Do nothing
            break;
    }
}

void Boss::render(SDL_Renderer* renderer) {
    // Draw boss as large red rectangle
    if (m_state == BossState::DYING) {
        // Flash during death
        Uint8 alpha = static_cast<Uint8>(128 + 127 * sin(m_deathTimer * 20));
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, alpha);
    } else {
        SDL_SetRenderDrawColor(renderer, 200, 0, 0, 255);
    }
    
    SDL_Rect rect = {
        static_cast<int>(m_x),
        static_cast<int>(m_y),
        static_cast<int>(m_width),
        static_cast<int>(m_height)
    };
    SDL_RenderFillRect(renderer, &rect);
}

void Boss::render(SDL_Renderer* renderer, SDL_Texture* texture, SDL_Rect* srcRect) {
    if (texture) {
        SDL_Rect dstRect = {
            static_cast<int>(m_x),
            static_cast<int>(m_y),
            static_cast<int>(m_width),
            static_cast<int>(m_height)
        };
        
        // Flash during death
        if (m_state == BossState::DYING) {
            Uint8 alpha = static_cast<Uint8>(128 + 127 * sin(m_deathTimer * 20));
            SDL_SetTextureAlphaMod(texture, alpha);
        }
        
        SDL_RenderCopy(renderer, texture, srcRect, &dstRect);
        
        // Reset alpha
        if (m_state == BossState::DYING) {
            SDL_SetTextureAlphaMod(texture, 255);
        }
    } else {
        render(renderer);
    }
}

void Boss::takeDamage(int damage) {
    if (m_state == BossState::FIGHTING) {
        m_health -= damage;
        if (m_health <= 0) {
            m_health = 0;
            m_state = BossState::DYING;
            m_deathTimer = 0.0f;
        }
    }
}

bool Boss::isOffScreen() const {
    return m_state == BossState::DEAD;
}
