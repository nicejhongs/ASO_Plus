#include "Game.h"
#include <cstdlib>
#include <ctime>

Game::Game() 
    : m_window(nullptr)
    , m_renderer(nullptr)
    , m_running(false)
    , m_enemySpawnTimer(0.0f)
    , m_lives(3)
    , m_score(0)
    , m_highScore(0)
{
}

Game::~Game() {
    clean();
}

bool Game::init(const char* title, int width, int height) {
    // SDL 초기화
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        SDL_Log("SDL 초기화 실패: %s", SDL_GetError());
        return false;
    }

    // 윈도우 생성
    m_window = SDL_CreateWindow(
        title,
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        width, height,
        SDL_WINDOW_SHOWN
    );

    if (!m_window) {
        SDL_Log("윈도우 생성 실패: %s", SDL_GetError());
        return false;
    }

    // 렌더러 생성
    m_renderer = SDL_CreateRenderer(m_window, -1, SDL_RENDERER_ACCELERATED);
    if (!m_renderer) {
        SDL_Log("렌더러 생성 실패: %s", SDL_GetError());
        return false;
    }

    // 랜덤 시드 초기화
    srand(static_cast<unsigned>(time(nullptr)));

    // 플레이어 생성 (화면 중앙 하단)
    m_player = std::make_unique<Player>(width / 2.0f, height - 80.0f);

    m_running = true;
    return true;
}

void Game::handleEvents() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            m_running = false;
        }
        else if (event.type == SDL_MOUSEMOTION) {
            // 마우스 이동 시 마우스 위치 업데이트
            m_player->setMousePosition(static_cast<float>(event.motion.x), 
                                       static_cast<float>(event.motion.y));
        }
        else if (event.type == SDL_MOUSEBUTTONDOWN) {
            // 마우스 클릭 시 총알 발사
            if (event.button.button == SDL_BUTTON_LEFT && m_player->canShoot()) {
                m_bullets.push_back(std::make_unique<Bullet>(
                    m_player->getX() + m_player->getWidth() / 2,
                    m_player->getY(),
                    Bullet::Owner::PLAYER
                ));
                m_player->resetShootTimer();
            }
        }
    }
}

void Game::update(float deltaTime) {
    // 플레이어 업데이트
    m_player->update(deltaTime);

    // 적 생성
    m_enemySpawnTimer += deltaTime;
    if (m_enemySpawnTimer >= m_enemySpawnInterval) {
        int windowWidth, windowHeight;
        SDL_GetWindowSize(m_window, &windowWidth, &windowHeight);
        
        float enemyX = static_cast<float>(rand() % (windowWidth - 40));
        m_enemies.push_back(std::make_unique<Enemy>(enemyX, -40.0f));
        m_enemySpawnTimer = 0.0f;
    }

    // 적 업데이트
    for (auto& enemy : m_enemies) {
        enemy->update(deltaTime);
    }

    // 총알 업데이트
    for (auto& bullet : m_bullets) {
        bullet->update(deltaTime);
    }

    // 화면 밖으로 나간 적 제거
    m_enemies.erase(
        std::remove_if(m_enemies.begin(), m_enemies.end(),
            [](const std::unique_ptr<Enemy>& enemy) {
                return enemy->isOffScreen();
            }),
        m_enemies.end()
    );

    // 화면 밖으로 나간 총알 제거
    m_bullets.erase(
        std::remove_if(m_bullets.begin(), m_bullets.end(),
            [](const std::unique_ptr<Bullet>& bullet) {
                return bullet->isOffScreen();
            }),
        m_bullets.end()
    );

    // 충돌 감지 (간단한 AABB)
    for (auto bulletIt = m_bullets.begin(); bulletIt != m_bullets.end();) {
        bool bulletRemoved = false;
        
        if ((*bulletIt)->getOwner() == Bullet::Owner::PLAYER) {
            for (auto enemyIt = m_enemies.begin(); enemyIt != m_enemies.end();) {
                // 충돌 체크
                if ((*bulletIt)->getX() < (*enemyIt)->getX() + (*enemyIt)->getWidth() &&
                    (*bulletIt)->getX() + (*bulletIt)->getWidth() > (*enemyIt)->getX() &&
                    (*bulletIt)->getY() < (*enemyIt)->getY() + (*enemyIt)->getHeight() &&
                    (*bulletIt)->getY() + (*bulletIt)->getHeight() > (*enemyIt)->getY()) {
                    
                    // 충돌 발생: 둘 다 제거 및 점수 증가
                    m_score += 10;
                    if (m_score > m_highScore) {
                        m_highScore = m_score;
                    }
                    enemyIt = m_enemies.erase(enemyIt);
                    bulletIt = m_bullets.erase(bulletIt);
                    bulletRemoved = true;
                    break;
                } else {
                    ++enemyIt;
                }
            }
        }
        
        if (!bulletRemoved) {
            ++bulletIt;
        }
    }
    
    // 플레이어와 적 충돌 체크
    checkPlayerEnemyCollision();
}

void Game::checkPlayerEnemyCollision() {
    for (auto enemyIt = m_enemies.begin(); enemyIt != m_enemies.end();) {
        // 플레이어와 적의 충돌 체크
        if (m_player->getX() < (*enemyIt)->getX() + (*enemyIt)->getWidth() &&
            m_player->getX() + m_player->getWidth() > (*enemyIt)->getX() &&
            m_player->getY() < (*enemyIt)->getY() + (*enemyIt)->getHeight() &&
            m_player->getY() + m_player->getHeight() > (*enemyIt)->getY()) {
            
            // 충돌 발생: 적 제거, 생명 감소
            enemyIt = m_enemies.erase(enemyIt);
            m_lives--;
            
            // 생명이 0이 되면 게임 오버 (리셋)
            if (m_lives <= 0) {
                m_lives = 3;
                m_score = 0;
                m_enemies.clear();
                m_bullets.clear();
                
                // 플레이어 위치 리셋
                int windowWidth, windowHeight;
                SDL_GetWindowSize(m_window, &windowWidth, &windowHeight);
                m_player = std::make_unique<Player>(windowWidth / 2.0f, windowHeight - 80.0f);
            }
        } else {
            ++enemyIt;
        }
    }
}

void Game::render() {
    // 화면 클리어 (검은색)
    SDL_SetRenderDrawColor(m_renderer, 0, 0, 0, 255);
    SDL_RenderClear(m_renderer);

    // 플레이어 렌더링
    m_player->render(m_renderer);

    // 적 렌더링
    for (auto& enemy : m_enemies) {
        enemy->render(m_renderer);
    }

    // 총알 렌더링
    for (auto& bullet : m_bullets) {
        bullet->render(m_renderer);
    }
    
    // UI 렌더링
    renderUI();

    // 화면에 표시
    SDL_RenderPresent(m_renderer);
}

void Game::renderUI() {
    // 왼쪽 위: 점수 표시 (간단한 막대로)
    SDL_SetRenderDrawColor(m_renderer, 255, 255, 255, 255);
    for (int i = 0; i < m_score / 10 && i < 50; i++) {
        SDL_Rect scoreRect = { 10 + i * 3, 10, 2, 15 };
        SDL_RenderFillRect(m_renderer, &scoreRect);
    }
    
    // 위쪽 가운데: 최고 점수 표시
    SDL_SetRenderDrawColor(m_renderer, 255, 215, 0, 255); // 금색
    for (int i = 0; i < m_highScore / 10 && i < 50; i++) {
        SDL_Rect highScoreRect = { 375 + i * 3, 10, 2, 15 };
        SDL_RenderFillRect(m_renderer, &highScoreRect);
    }
    
    // 오른쪽 위: 남은 생명 표시 (작은 우주선 모양)
    SDL_SetRenderDrawColor(m_renderer, 0, 150, 255, 255);
    for (int i = 0; i < m_lives; i++) {
        SDL_Rect lifeRect = { 780 - i * 25, 10, 20, 20 };
        SDL_RenderFillRect(m_renderer, &lifeRect);
    }
}

void Game::clean() {
    if (m_renderer) {
        SDL_DestroyRenderer(m_renderer);
        m_renderer = nullptr;
    }

    if (m_window) {
        SDL_DestroyWindow(m_window);
        m_window = nullptr;
    }

    SDL_Quit();
}
