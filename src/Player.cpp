#include "Player.h"
#include <cmath>

Player::Player(float x, float y)
    : m_x(x)
    , m_y(y)
    , m_width(80.0f)  // 256x256 스프라이트를 80x80으로 표시
    , m_height(80.0f)
    , m_speed(300.0f)
    , m_shootCooldown(0.2f)
    , m_shootTimer(0.0f)
    , m_mouseX(x)
    , m_mouseY(y)
{
}

Player::~Player() {
}

void Player::setMousePosition(float mouseX, float mouseY) {
    m_mouseX = mouseX;
    m_mouseY = mouseY;
}

void Player::update(float deltaTime) {
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
        
        // 화면 경계 체크
        if (m_x < 0) m_x = 0;
        if (m_x > 800 - m_width) m_x = 800 - m_width;
        if (m_y < 0) m_y = 0;
        if (m_y > 600 - m_height) m_y = 600 - m_height;
    }
    
    // 발사 쿨다운 타이머
    if (m_shootTimer > 0) {
        m_shootTimer -= deltaTime;
    }
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
