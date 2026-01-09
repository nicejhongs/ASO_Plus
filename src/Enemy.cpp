#include "Enemy.h"

Enemy::Enemy(float x, float y)
    : m_x(x)
    , m_y(y)
    , m_width(60.0f)  // 크기 증가
    , m_height(60.0f)
    , m_speed(150.0f)
{
}

Enemy::~Enemy() {
}

void Enemy::update(float deltaTime) {
    // 아래로 이동
    m_y += m_speed * deltaTime;
}

void Enemy::render(SDL_Renderer* renderer) {
    // 적을 빨간색 사각형으로 그리기
    SDL_SetRenderDrawColor(renderer, 255, 50, 50, 255);
    SDL_Rect rect = {
        static_cast<int>(m_x),
        static_cast<int>(m_y),
        static_cast<int>(m_width),
        static_cast<int>(m_height)
    };
    SDL_RenderFillRect(renderer, &rect);
}

bool Enemy::isOffScreen() const {
    return m_y > 600; // 화면 높이보다 아래로 나가면 제거
}

void Enemy::render(SDL_Renderer* renderer, SDL_Texture* texture, SDL_Rect* srcRect) {
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
