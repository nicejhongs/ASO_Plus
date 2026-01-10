#include "Player.h"
#include <cmath>

Player::Player(float x, float y)
    : m_x(x)
    , m_y(y)
    , m_width(80.0f)
    , m_height(80.0f)
    , m_speed(300.0f)
    , m_shootCooldown(0.5f)  // Slower auto-fire
    , m_shootTimer(0.0f)
    , m_mouseX(x)
    , m_mouseY(y)
    , m_speedLevel(0)
    , m_laserLevel(0)
    , m_missileLevel(0)
    , m_speedCount(0)
    , m_laserCount(0)
    , m_missileCount(0)
    , m_energy(16)
    , m_maxEnergy(16)
    , m_keepSpeed(false)
    , m_keepLaser(false)
    , m_keepMissile(false)
    , m_movementState(MovementState::STOP)
    , m_lastY(y)
    , m_lastX(x)
{
}

Player::~Player() {
}

void Player::setMousePosition(float mouseX, float mouseY) {
    m_mouseX = mouseX;
    m_mouseY = mouseY;
}

void Player::update(float deltaTime) {
    // Store last position to detect movement direction
    float previousY = m_y;
    float previousX = m_x;
    
    // 플레이어 중심 계산
    float playerCenterX = m_x + m_width / 2;
    float playerCenterY = m_y + m_height / 2;
    
    // Calculate distance between mouse and player
    float dx = m_mouseX - playerCenterX;
    float dy = m_mouseY - playerCenterY;
    float distance = sqrt(dx * dx + dy * dy);
    
    // Set deadzone (don't move if mouse is too close to player)
    const float deadZone = 20.0f;
    
    if (distance > deadZone) {
        // 방향 벡터 정규화
        float dirX = dx / distance;
        float dirY = dy / distance;
        
        // 일정한 속도로 이동
        m_x += dirX * m_speed * deltaTime;
        m_y += dirY * m_speed * deltaTime;
        
        // 화면 경계 체크 (최소값만)
        if (m_x < 0) m_x = 0;
        if (m_y < 0) m_y = 0;
    }
    
    // Update movement state based on position change
    const float movementThreshold = 1.0f;
    float deltaX = m_x - previousX;
    float deltaY = m_y - previousY;
    
    // Prioritize horizontal movement over vertical
    if (fabs(deltaX) > fabs(deltaY)) {
        if (deltaX < -movementThreshold) {
            m_movementState = MovementState::LEFT;  // Moving left
        } else if (deltaX > movementThreshold) {
            m_movementState = MovementState::RIGHT;  // Moving right
        } else {
            m_movementState = MovementState::STOP;
        }
    } else {
        if (deltaY < -movementThreshold) {
            m_movementState = MovementState::FORWARD;  // Moving up (forward in shooter games)
        } else if (deltaY > movementThreshold) {
            m_movementState = MovementState::BACKWARD;  // Moving down (backward)
        } else {
            m_movementState = MovementState::STOP;  // Not moving
        }
    }
    
    m_lastY = m_y;
    m_lastX = m_x;

    // 발사 쿨다운 업데이트
    if (m_shootTimer > 0) {
        m_shootTimer -= deltaTime;
    }
}

void Player::clampToScreen(int screenWidth, int screenHeight) {
    if (m_x < 0) m_x = 0;
    if (m_x > screenWidth - m_width) m_x = screenWidth - m_width;
    if (m_y < 0) m_y = 0;
    if (m_y > screenHeight - m_height) m_y = screenHeight - m_height;
}

void Player::render(SDL_Renderer* renderer) {
    // 플레이어를 파란색 사각형으로 그리기
    SDL_SetRenderDrawColor(renderer, 0, 150, 255, 255);
    SDL_Rect rect = {
        static_cast<int>(m_x),
        static_cast<int>(m_y),
        static_cast<int>(m_width),
        static_cast<int>(m_height)
    };
    SDL_RenderFillRect(renderer, &rect);
}

void Player::render(SDL_Renderer* renderer, SDL_Texture* texture, SDL_Rect* srcRect) {
    if (texture) {
        SDL_Rect dstRect = {
            static_cast<int>(m_x),
            static_cast<int>(m_y),
            static_cast<int>(m_width),
            static_cast<int>(m_height)
        };
        // srcRect가 nullptr이면 전체 텍스처 사용
        SDL_RenderCopy(renderer, texture, srcRect, &dstRect);
    } else {
        // 폴백: 기본 렌더링
        render(renderer);
    }
}

bool Player::canShoot() const {
    return m_shootTimer <= 0;
}

void Player::resetShootTimer() {
    m_shootTimer = m_shootCooldown;
}

// Collection system - need 3 items to upgrade
void Player::collectSpeed() {
    m_speedCount++;
    if (m_speedCount >= 3) {
        upgradeSpeed();
        m_speedCount = 0;
    }
}

void Player::collectLaser() {
    m_laserCount++;
    if (m_laserCount >= 3) {
        upgradeLaser();
        m_laserCount = 0;
    }
}

void Player::collectMissile() {
    m_missileCount++;
    if (m_missileCount >= 3) {
        upgradeMissile();
        m_missileCount = 0;
    }
}

// Direct upgrade methods
void Player::upgradeSpeed() {
    if (m_speedLevel < 3) {
        m_speedLevel++;
        m_speed = 300.0f + (m_speedLevel * 80.0f);  // 300, 380, 460, 540
    }
}

void Player::upgradeLaser() {
    if (m_laserLevel < 3) {
        m_laserLevel++;
        // Laser level affects bullet count in game logic
    }
}

void Player::upgradeMissile() {
    if (m_missileLevel < 4) {
        m_missileLevel++;
        // Missile level affects ground attack in game logic
    }
}

// Downgrade methods
void Player::downgradeSpeed() {
    if (m_speedLevel > 0) {
        m_speedLevel--;
        m_speed = 300.0f + (m_speedLevel * 80.0f);
    }
    m_speedCount = 0;
    m_keepSpeed = false;
}

void Player::downgradeLaser() {
    if (m_laserLevel > 0) {
        m_laserLevel--;
    }
    m_laserCount = 0;
    m_keepLaser = false;
}

void Player::downgradeMissile() {
    if (m_missileLevel > 0) {
        m_missileLevel--;
    }
    m_missileCount = 0;
    m_keepMissile = false;
}

// Energy system
void Player::addEnergy(int amount) {
    m_energy += amount;
    if (m_energy > m_maxEnergy) {
        m_energy = m_maxEnergy;
    }
}

void Player::removeEnergy(int amount) {
    m_energy -= amount;
    if (m_energy < 0) {
        m_energy = 0;
    }
}

void Player::increaseMaxEnergy() {
    if (m_maxEnergy < 24) {
        m_maxEnergy++;
        m_energy++;
    }
}

// Take damage (returns true if player should explode)
bool Player::takeDamage(int amount) {
    removeEnergy(amount);
    return (m_energy <= 0);
}

// Reset on death
void Player::resetOnDeath() {
    // Reset power levels unless kept
    if (!m_keepSpeed) {
        m_speedLevel = 0;
        m_speedCount = 0;
        m_speed = 300.0f;
    }
    if (!m_keepLaser) {
        m_laserLevel = 0;
        m_laserCount = 0;
    }
    if (!m_keepMissile) {
        m_missileLevel = 0;
        m_missileCount = 0;
    }
    
    // Reset energy to half
    m_energy = m_maxEnergy / 2;
}
