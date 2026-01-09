#include "Game.h"
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <fstream>
#include <algorithm>

#ifdef _WIN32
#include <windows.h>
#endif

Game::Game() 
    : m_window(nullptr)
    , m_renderer(nullptr)
    , m_running(false)
    , m_enemySpawnTimer(0.0f)
    , m_gameState(GameState::MENU)
    , m_stateTimer(0.0f)
    , m_lives(3)
    , m_score(0)
    , m_highScore(0)
    , m_shootSound(nullptr)
    , m_explosionSound(nullptr)
    , m_spriteSheet(nullptr)
{
    loadHighScores();
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

    // SDL_mixer 초기화
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        SDL_Log("SDL_mixer 초기화 실패: %s", Mix_GetError());
        // 계속 진행 (사운드 없이)
    }
    
    // 사운드 파일 로드
    m_shootSound = Mix_LoadWAV("assets/shoot.wav");
    if (!m_shootSound) {
        SDL_Log("shoot.wav 로드 실패: %s", Mix_GetError());
    }
    
    m_explosionSound = Mix_LoadWAV("assets/explosion.wav");
    if (!m_explosionSound) {
        SDL_Log("explosion.wav 로드 실패: %s", Mix_GetError());
    }

    // 스프라이트 시트 로드
    int imgFlags = IMG_INIT_PNG;
    if (!(IMG_Init(imgFlags) & imgFlags)) {
        SDL_Log("SDL_image 초기화 실패: %s", IMG_GetError());
    }
    
    SDL_Surface* surface = IMG_Load("assets/sprites.png");
    if (surface) {
        m_spriteSheet = SDL_CreateTextureFromSurface(m_renderer, surface);
        SDL_FreeSurface(surface);
        
        if (m_spriteSheet) {
            // 스프라이트 시트에서 플레이어와 적의 위치 정의
            // 플레이어 스프라이트 (왼쪽 상단 우주선, 크기 약 64x64)
            m_playerSprite = {0, 0, 64, 64};
            
            // 적 스프라이트 (오른쪽 적 우주선, 크기 약 64x64)
            m_enemySprite = {640, 0, 64, 64};
        }
    } else {
        SDL_Log("sprites.png 로드 실패: %s", IMG_GetError());
    }

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
            if (m_gameState == GameState::PLAYING) {
                // 마우스 이동 시 마우스 위치 업데이트
                m_player->setMousePosition(static_cast<float>(event.motion.x), 
                                           static_cast<float>(event.motion.y));
            }
        }
        else if (event.type == SDL_MOUSEBUTTONDOWN) {
            if (event.button.button == SDL_BUTTON_LEFT) {
                // MENU 또는 GAME_OVER 상태에서 클릭하면 카운트다운 시작
                if (m_gameState == GameState::MENU || m_gameState == GameState::GAME_OVER) {
                    m_gameState = GameState::COUNTDOWN;
                    m_stateTimer = 5.0f;
                    m_lives = 3;
                    m_score = 0;
                    m_enemies.clear();
                    m_bullets.clear();
                    int windowWidth, windowHeight;
                    SDL_GetWindowSize(m_window, &windowWidth, &windowHeight);
                    m_player = std::make_unique<Player>(windowWidth / 2.0f, windowHeight - 80.0f);
                }
                // PLAYING 상태에서는 총알 발사
                else if (m_gameState == GameState::PLAYING && m_player->canShoot()) {
                    m_bullets.push_back(std::make_unique<Bullet>(
                        m_player->getX() + m_player->getWidth() / 2,
                        m_player->getY(),
                        Bullet::Owner::PLAYER
                    ));
                    m_player->resetShootTimer();
                    
                    // 발사 사운드 재생
                    if (m_shootSound) {
                        Mix_PlayChannel(-1, m_shootSound, 0);
                    } else {
                        // 폴백: Windows 비프음
                        #ifdef _WIN32
                        Beep(800, 50);
                        #endif
                    }
                }
            }
        }
    }
}

void Game::update(float deltaTime) {
    // 게임 상태에 따른 처리
    if (m_gameState == GameState::MENU) {
        // 메뉴 상태에서는 아무것도 업데이트하지 않음
        return;
    }
    else if (m_gameState == GameState::COUNTDOWN) {
        m_stateTimer -= deltaTime;
        if (m_stateTimer <= 0) {
            m_gameState = GameState::PLAYING;
        }
        return;
    }
    else if (m_gameState == GameState::GAME_OVER) {
        // 게임 오버 상태에서는 아무것도 업데이트하지 않음
        return;
    }
    
    // PLAYING 상태에서만 게임 로직 실행
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
            
            // 폭발음 재생
            if (m_explosionSound) {
                Mix_PlayChannel(-1, m_explosionSound, 0);
            } else {
                // 폴백: Windows 비프음
                #ifdef _WIN32
                Beep(300, 200);
                #endif
            }
            
            // 생명이 0이 되면 게임 오버
            if (m_lives <= 0) {
                // 화면을 빨간색으로 플래시 (폭발 효과)
                SDL_SetRenderDrawColor(m_renderer, 255, 0, 0, 255);
                SDL_RenderClear(m_renderer);
                SDL_RenderPresent(m_renderer);
                SDL_Delay(300); // 0.3초 대기
                
                // 화면을 검은색으로 지우기
                SDL_SetRenderDrawColor(m_renderer, 0, 0, 0, 255);
                SDL_RenderClear(m_renderer);
                SDL_RenderPresent(m_renderer);
                SDL_Delay(200); // 0.2초 대기
                
                // 최고 점수 저장
                addHighScore(m_score);
                
                // 게임 오버 상태로 전환
                m_gameState = GameState::GAME_OVER;
                m_enemies.clear();
                m_bullets.clear();
            }
            else {
                // 생명이 남아있으면 카운트다운 후 재시작
                m_gameState = GameState::COUNTDOWN;
                m_stateTimer = 3.0f; // 3초 카운트다운
                m_enemies.clear();
                m_bullets.clear();
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

    // 게임 상태에 따른 렌더링
    if (m_gameState == GameState::MENU) {
        renderMenu();
    }
    else if (m_gameState == GameState::COUNTDOWN) {
        renderCountdown();
    }
    else if (m_gameState == GameState::PLAYING) {
        // 플레이어 렌더링 (스프라이트 사용)
        if (m_spriteSheet) {
            m_player->render(m_renderer, m_spriteSheet, &m_playerSprite);
        } else {
            m_player->render(m_renderer);
        }

        // 적 렌더링 (스프라이트 사용)
        for (auto& enemy : m_enemies) {
            if (m_spriteSheet) {
                enemy->render(m_renderer, m_spriteSheet, &m_enemySprite);
            } else {
                enemy->render(m_renderer);
            }
        }

        // 총알 렌더링
        for (auto& bullet : m_bullets) {
            bullet->render(m_renderer);
        }
        
        // UI 렌더링
        renderUI();
    }
    else if (m_gameState == GameState::GAME_OVER) {
        renderGameOver();
    }

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
    // 사운드 해제
    if (m_shootSound) {
        Mix_FreeChunk(m_shootSound);
        m_shootSound = nullptr;
    }
    if (m_explosionSound) {
        Mix_FreeChunk(m_explosionSound);
        m_explosionSound = nullptr;
    }
    
    Mix_CloseAudio();
    
    // 텍스처 해제
    if (m_spriteSheet) {
        SDL_DestroyTexture(m_spriteSheet);
        m_spriteSheet = nullptr;
    }
    
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

void Game::renderMenu() {
    // 타이틀 텍스트 (간단한 박스로 표현)
    SDL_SetRenderDrawColor(m_renderer, 255, 255, 255, 255);
    SDL_Rect titleRect = { 250, 150, 300, 50 };
    SDL_RenderDrawRect(m_renderer, &titleRect);
    
    // "ASO PLUS" 텍스트를 막대로 표현
    for (int i = 0; i < 8; i++) {
        SDL_Rect letterRect = { 280 + i * 50, 160, 30, 30 };
        SDL_RenderFillRect(m_renderer, &letterRect);
    }
    
    // 시작 안내 메시지
    SDL_SetRenderDrawColor(m_renderer, 0, 255, 0, 255);
    SDL_Rect startRect = { 300, 300, 200, 30 };
    SDL_RenderFillRect(m_renderer, &startRect);
    
    // "CLICK TO START" 표시
    for (int i = 0; i < 3; i++) {
        SDL_Rect clickRect = { 320 + i * 40, 310, 30, 10 };
        SDL_RenderFillRect(m_renderer, &clickRect);
    }
}

void Game::renderCountdown() {
    // 카운트다운 숫자 표시
    int countdown = static_cast<int>(m_stateTimer) + 1;
    SDL_SetRenderDrawColor(m_renderer, 255, 255, 0, 255);
    
    // 큰 숫자로 표시
    drawNumber(countdown, 350, 250, 10);
    
    // "GET READY" 메시지
    SDL_Rect readyRect = { 300, 150, 200, 40 };
    SDL_RenderFillRect(m_renderer, &readyRect);
}

void Game::renderGameOver() {
    // "GAME OVER" 텍스트
    SDL_SetRenderDrawColor(m_renderer, 255, 0, 0, 255);
    SDL_Rect gameOverRect = { 250, 100, 300, 50 };
    SDL_RenderFillRect(m_renderer, &gameOverRect);
    
    // 현재 점수 표시
    SDL_SetRenderDrawColor(m_renderer, 255, 255, 255, 255);
    drawNumber(m_score, 350, 200, 5);
    
    // 최고 점수 목록 표시
    SDL_SetRenderDrawColor(m_renderer, 255, 215, 0, 255);
    int y = 280;
    for (size_t i = 0; i < m_highScores.size() && i < 10; i++) {
        drawNumber(static_cast<int>(i + 1), 200, y, 2);
        drawNumber(m_highScores[i], 350, y, 3);
        y += 30;
    }
    
    // "CLICK TO RESTART" 메시지
    SDL_SetRenderDrawColor(m_renderer, 0, 255, 0, 255);
    SDL_Rect restartRect = { 280, 550, 240, 30 };
    SDL_RenderFillRect(m_renderer, &restartRect);
}

void Game::drawNumber(int number, int x, int y, int scale) {
    // 7-세그먼트 스타일로 숫자 그리기
    if (number == 0) number = 0; // 음수 방지
    
    std::string numStr = std::to_string(number);
    int digitWidth = 8 * scale;
    int spacing = 2 * scale;
    
    for (size_t i = 0; i < numStr.length(); i++) {
        int digit = numStr[i] - '0';
        int dx = x + i * (digitWidth + spacing);
        
        // 간단한 블록 스타일 숫자
        for (int row = 0; row < 5; row++) {
            for (int col = 0; col < 3; col++) {
                bool fill = false;
                // 각 숫자의 패턴 (간단화)
                if (digit == 0) fill = (row == 0 || row == 4 || col == 0 || col == 2);
                else if (digit == 1) fill = (col == 2);
                else if (digit == 2) fill = (row == 0 || row == 2 || row == 4 || (row == 1 && col == 2) || (row == 3 && col == 0));
                else if (digit == 3) fill = (row == 0 || row == 2 || row == 4 || col == 2);
                else if (digit == 4) fill = ((row <= 2 && col == 0) || col == 2 || row == 2);
                else if (digit == 5) fill = (row == 0 || row == 2 || row == 4 || (row == 1 && col == 0) || (row == 3 && col == 2));
                else if (digit == 6) fill = (row == 0 || row == 2 || row == 4 || col == 0 || (row >= 2 && col == 2));
                else if (digit == 7) fill = (row == 0 || col == 2);
                else if (digit == 8) fill = true;
                else if (digit == 9) fill = (row == 0 || row == 2 || col == 2 || (row <= 2 && col == 0));
                
                if (fill) {
                    SDL_Rect digitRect = { dx + col * scale * 2, y + row * scale * 2, scale * 2, scale * 2 };
                    SDL_RenderFillRect(m_renderer, &digitRect);
                }
            }
        }
    }
}

void Game::saveHighScores() {
    std::ofstream file("highscores.txt");
    if (file.is_open()) {
        for (int score : m_highScores) {
            file << score << std::endl;
        }
        file.close();
    }
}

void Game::loadHighScores() {
    std::ifstream file("highscores.txt");
    if (file.is_open()) {
        int score;
        while (file >> score && m_highScores.size() < 10) {
            m_highScores.push_back(score);
        }
        file.close();
    }
    
    if (!m_highScores.empty()) {
        m_highScore = m_highScores[0];
    }
}

void Game::addHighScore(int score) {
    m_highScores.push_back(score);
    std::sort(m_highScores.begin(), m_highScores.end(), std::greater<int>());
    
    // 상위 10개만 유지
    if (m_highScores.size() > 10) {
        m_highScores.resize(10);
    }
    
    if (!m_highScores.empty()) {
        m_highScore = m_highScores[0];
    }
    
    saveHighScores();
}
