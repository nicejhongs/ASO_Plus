#include "Game.h"
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <fstream>
#include <algorithm>

#ifdef _WIN32
#include <windows.h>
#endif

// 실행 파일 경로를 기준으로 리소스 경로 생성
std::string getResourcePath(const std::string& filename) {
#ifdef _WIN32
    char buffer[MAX_PATH];
    GetModuleFileNameA(NULL, buffer, MAX_PATH);
    std::string::size_type pos = std::string(buffer).find_last_of("\\/");
    std::string exePath = std::string(buffer).substr(0, pos);
    return exePath + "\\assets\\" + filename;
#else
    return "assets/" + filename;
#endif
}

Game::Game() 
    : m_window(nullptr)
    , m_renderer(nullptr)
    , m_running(false)
    , m_mouseGrabbed(true)  // Locked by default
    , m_enemySpawnTimer(0.0f)
    , m_gameState(GameState::MENU)
    , m_stateTimer(0.0f)
    , m_lives(3)
    , m_score(0)
    , m_highScore(0)
    , m_shootSound(nullptr)
    , m_explosionSound(nullptr)
    , m_bgMusic(nullptr)
    , m_backgroundTexture(nullptr)
    , m_backgroundY1(0.0f)
    , m_backgroundY2(-600.0f)  // 화면 높이만큼 위에
    , m_backgroundScrollSpeed(50.0f)  // 픽셀/초
    , m_playerTexture(nullptr)
    , m_enemyTexture(nullptr)
    , m_titleFont(nullptr)
    , m_uiFont(nullptr)
    , m_subtitleFont(nullptr)
    , m_titleTexture(nullptr)
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
    
    // SDL_ttf 초기화
    if (TTF_Init() == -1) {
        SDL_Log("TTF 초기화 실패: %s", TTF_GetError());
        // 계속 진행 (폰트 없이)
    }

    // 랜덤 시드 초기화
    srand(static_cast<unsigned>(time(nullptr)));

    // SDL_mixer 초기화
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        SDL_Log("SDL_mixer 초기화 실패: %s", Mix_GetError());
        // 계속 진행 (사운드 없이)
    }
    
    // 사운드 파일 로드 (WAV 형식, Mix_Chunk 사용)
    std::string shootPath = getResourcePath("shot_01.wav");
    m_shootSound = Mix_LoadWAV(shootPath.c_str());
    if (!m_shootSound) {
        SDL_Log("INFO: shot_01.wav 로드 실패: %s (path: %s)", Mix_GetError(), shootPath.c_str());
    } else {
        SDL_Log("INFO: shot_01.wav 로드 성공: %s", shootPath.c_str());
    }
    
    std::string explosionPath = getResourcePath("exlposion_01.wav");
    m_explosionSound = Mix_LoadWAV(explosionPath.c_str());
    if (!m_explosionSound) {
        SDL_Log("INFO: exlposion_01.wav 로드 실패: %s (path: %s)", Mix_GetError(), explosionPath.c_str());
    } else {
        SDL_Log("INFO: exlposion_01.wav 로드 성공: %s", explosionPath.c_str());
    }
    
    // 배경음악 로드 (WAV 형식으로 Mix_Music 사용)
    std::string bgMusicPath = getResourcePath("aso_plus_opening.wav");
    m_bgMusic = Mix_LoadMUS(bgMusicPath.c_str());
    if (!m_bgMusic) {
        SDL_Log("INFO: aso_plus_opening.wav 로드 실패: %s (path: %s)", Mix_GetError(), bgMusicPath.c_str());
    } else {
        SDL_Log("INFO: aso_plus_opening.wav 로드 성공: %s", bgMusicPath.c_str());
        // 배경음악 무한 반복 재생 (-1 = loop)
        Mix_PlayMusic(m_bgMusic, -1);
        Mix_VolumeMusic(64); // 볼륨 50% (0-128)
    }

    // 스프라이트 이미지 로드 (PNG 투명도 지원)
    int imgFlags = IMG_INIT_PNG;
    if (!(IMG_Init(imgFlags) & imgFlags)) {
        SDL_Log("SDL_image 초기화 실패: %s", IMG_GetError());
    }
    
    // 배경화면 로드 (background.png)
    std::string bgPath = getResourcePath("background.png");
    SDL_Surface* bgSurface = IMG_Load(bgPath.c_str());
    if (bgSurface) {
        m_backgroundTexture = SDL_CreateTextureFromSurface(m_renderer, bgSurface);
        SDL_FreeSurface(bgSurface);
        
        if (m_backgroundTexture) {
            SDL_Log("INFO: background.png 로드 성공: %s", bgPath.c_str());
        }
    } else {
        SDL_Log("INFO: background.png 로드 실패: %s (path: %s)", IMG_GetError(), bgPath.c_str());
    }
    
    // 플레이어 스프라이트 로드 (ship_01.png)
    std::string playerSpritePath = getResourcePath("ship_01.png");
    SDL_Surface* playerSurface = IMG_Load(playerSpritePath.c_str());
    if (playerSurface) {
        // 흰색 배경을 투명으로 처리 (RGB: 255, 255, 255)
        SDL_SetColorKey(playerSurface, SDL_TRUE, SDL_MapRGB(playerSurface->format, 255, 255, 255));
        
        m_playerTexture = SDL_CreateTextureFromSurface(m_renderer, playerSurface);
        SDL_FreeSurface(playerSurface);
        
        if (m_playerTexture) {
            SDL_Log("INFO: ship_01.png 로드 성공: %s", playerSpritePath.c_str());
            SDL_SetTextureBlendMode(m_playerTexture, SDL_BLENDMODE_BLEND);
        }
    } else {
        SDL_Log("INFO: ship_01.png 로드 실패: %s (path: %s)", IMG_GetError(), playerSpritePath.c_str());
    }
    
    // 적 스프라이트 로드 (enemy_02.png)
    std::string enemySpritePath = getResourcePath("enemy_02.png");
    SDL_Surface* enemySurface = IMG_Load(enemySpritePath.c_str());
    if (enemySurface) {
        // 흰색 배경을 투명으로 처리 (RGB: 255, 255, 255)
        SDL_SetColorKey(enemySurface, SDL_TRUE, SDL_MapRGB(enemySurface->format, 255, 255, 255));
        
        m_enemyTexture = SDL_CreateTextureFromSurface(m_renderer, enemySurface);
        SDL_FreeSurface(enemySurface);
        
        if (m_enemyTexture) {
            SDL_Log("INFO: enemy_02.png 로드 성공: %s", enemySpritePath.c_str());
            SDL_SetTextureBlendMode(m_enemyTexture, SDL_BLENDMODE_BLEND);
        }
    } else {
        SDL_Log("INFO: enemy_02.png 로드 실패: %s (path: %s)", IMG_GetError(), enemySpritePath.c_str());
    }
    
    // 폰트 로드 (Windows 기본 폰트 사용)
    m_titleFont = TTF_OpenFont("C:\\Windows\\Fonts\\arial.ttf", 72);
    if (!m_titleFont) {
        SDL_Log("INFO: 폰트 로드 실패: %s", TTF_GetError());
    } else {
        // "ASO PLUS" 텍스트 렌더링
        SDL_Color titleColor = {255, 255, 255, 255};  // 흰색
        SDL_Surface* titleSurface = TTF_RenderText_Blended(m_titleFont, "ASO PLUS", titleColor);
        if (titleSurface) {
            m_titleTexture = SDL_CreateTextureFromSurface(m_renderer, titleSurface);
            SDL_FreeSurface(titleSurface);
            if (m_titleTexture) {
                SDL_Log("INFO: 타이틀 텍스트 생성 성공");
            }
        }
    }
    
    // UI 폰트 로드 (점수, 카운트다운용)
    m_uiFont = TTF_OpenFont("C:\\Windows\\Fonts\\arial.ttf", 48);
    if (!m_uiFont) {
        SDL_Log("INFO: UI 폰트 로드 실패: %s", TTF_GetError());
    }
    
    // 부제목 폰트 로드
    m_subtitleFont = TTF_OpenFont("C:\\Windows\\Fonts\\arial.ttf", 32);
    if (!m_subtitleFont) {
        SDL_Log("INFO: 부제목 폰트 로드 실패: %s", TTF_GetError());
    }

    // 플레이어 생성 (화면 중앙 하단)
    m_player = std::make_unique<Player>(width / 2.0f, height - 80.0f);
    
    // Confine mouse to window
    SDL_SetWindowGrab(m_window, SDL_TRUE);
    SDL_Log("INFO: Mouse grab enabled (ESC to release)");

    m_running = true;
    return true;
}

void Game::handleEvents() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            m_running = false;
        }
        else if (event.type == SDL_KEYDOWN) {
            // ESC to toggle mouse grab
            if (event.key.keysym.sym == SDLK_ESCAPE) {
                m_mouseGrabbed = !m_mouseGrabbed;
                SDL_SetWindowGrab(m_window, m_mouseGrabbed ? SDL_TRUE : SDL_FALSE);
                SDL_Log("INFO: Mouse grab %s", m_mouseGrabbed ? "enabled" : "disabled");
            }
            // Any key to start from MENU or GAME_OVER
            else if (m_gameState == GameState::MENU || m_gameState == GameState::GAME_OVER) {
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
        }
        else if (event.type == SDL_MOUSEMOTION) {
            if (m_gameState == GameState::PLAYING) {
                // Update mouse position on motion
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
    // 배경 스크롤
    m_backgroundY1 += m_backgroundScrollSpeed * deltaTime;
    m_backgroundY2 += m_backgroundScrollSpeed * deltaTime;
    
    // 배경이 화면 밖으로 나가면 다시 위로
    if (m_backgroundY1 >= 600) {
        m_backgroundY1 = m_backgroundY2 - 600;
    }
    if (m_backgroundY2 >= 600) {
        m_backgroundY2 = m_backgroundY1 - 600;
    }
    
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
                    
                    // 폭발음 재생
                    if (m_explosionSound) {
                        Mix_PlayChannel(-1, m_explosionSound, 0);
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
    
    // 배경 그리기 (가장 먼저)
    if (m_backgroundTexture) {
        SDL_Rect bg1 = {0, static_cast<int>(m_backgroundY1), 800, 600};
        SDL_Rect bg2 = {0, static_cast<int>(m_backgroundY2), 800, 600};
        SDL_RenderCopy(m_renderer, m_backgroundTexture, nullptr, &bg1);
        SDL_RenderCopy(m_renderer, m_backgroundTexture, nullptr, &bg2);
    }

    // 게임 상태에 따른 렌더링
    if (m_gameState == GameState::MENU) {
        renderMenu();
    }
    else if (m_gameState == GameState::COUNTDOWN) {
        renderCountdown();
    }
    else if (m_gameState == GameState::PLAYING) {
        // 플레이어 렌더링 (단일 텍스처 사용)
        if (m_playerTexture) {
            m_player->render(m_renderer, m_playerTexture, nullptr);
        } else {
            m_player->render(m_renderer);
        }

        // 적 렌더링 (단일 텍스처 사용)
        for (auto& enemy : m_enemies) {
            if (m_enemyTexture) {
                enemy->render(m_renderer, m_enemyTexture, nullptr);
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
    if (!m_uiFont) return;
    
    // 왼쪽 위: 점수 표시
    SDL_Color scoreColor = {255, 255, 255, 255};  // 흰색
    std::string scoreText = "SCORE: " + std::to_string(m_score);
    SDL_Surface* scoreSurface = TTF_RenderText_Blended(m_uiFont, scoreText.c_str(), scoreColor);
    if (scoreSurface) {
        SDL_Texture* scoreTexture = SDL_CreateTextureFromSurface(m_renderer, scoreSurface);
        if (scoreTexture) {
            SDL_Rect scoreRect = {10, 10, scoreSurface->w, scoreSurface->h};
            SDL_RenderCopy(m_renderer, scoreTexture, nullptr, &scoreRect);
            SDL_DestroyTexture(scoreTexture);
        }
        SDL_FreeSurface(scoreSurface);
    }
    
    // 위쪽 가운데: 최고 점수 표시
    SDL_Color highScoreColor = {255, 215, 0, 255}; // 금색
    std::string highScoreText = "HI: " + std::to_string(m_highScore);
    SDL_Surface* highScoreSurface = TTF_RenderText_Blended(m_uiFont, highScoreText.c_str(), highScoreColor);
    if (highScoreSurface) {
        SDL_Texture* highScoreTexture = SDL_CreateTextureFromSurface(m_renderer, highScoreSurface);
        if (highScoreTexture) {
            SDL_Rect highScoreRect = {(800 - highScoreSurface->w) / 2, 10, highScoreSurface->w, highScoreSurface->h};
            SDL_RenderCopy(m_renderer, highScoreTexture, nullptr, &highScoreRect);
            SDL_DestroyTexture(highScoreTexture);
        }
        SDL_FreeSurface(highScoreSurface);
    }
    
    // 오른쪽 위: 남은 생명 표시
    SDL_Color livesColor = {0, 150, 255, 255};
    std::string livesText = "LIVES: " + std::to_string(m_lives);
    SDL_Surface* livesSurface = TTF_RenderText_Blended(m_uiFont, livesText.c_str(), livesColor);
    if (livesSurface) {
        SDL_Texture* livesTexture = SDL_CreateTextureFromSurface(m_renderer, livesSurface);
        if (livesTexture) {
            SDL_Rect livesRect = {800 - livesSurface->w - 10, 10, livesSurface->w, livesSurface->h};
            SDL_RenderCopy(m_renderer, livesTexture, nullptr, &livesRect);
            SDL_DestroyTexture(livesTexture);
        }
        SDL_FreeSurface(livesSurface);
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
    if (m_bgMusic) {
        Mix_HaltMusic();
        Mix_FreeMusic(m_bgMusic);
        m_bgMusic = nullptr;
    }
    
    Mix_CloseAudio();
    
    // 텍스처 해제
    if (m_backgroundTexture) {
        SDL_DestroyTexture(m_backgroundTexture);
        m_backgroundTexture = nullptr;
    }
    if (m_playerTexture) {
        SDL_DestroyTexture(m_playerTexture);
        m_playerTexture = nullptr;
    }
    if (m_enemyTexture) {
        SDL_DestroyTexture(m_enemyTexture);
        m_enemyTexture = nullptr;
    }
    if (m_titleTexture) {
        SDL_DestroyTexture(m_titleTexture);
        m_titleTexture = nullptr;
    }
    
    // 폰트 해제
    if (m_titleFont) {
        TTF_CloseFont(m_titleFont);
        m_titleFont = nullptr;
    }
    if (m_uiFont) {
        TTF_CloseFont(m_uiFont);
        m_uiFont = nullptr;
    }
    if (m_subtitleFont) {
        TTF_CloseFont(m_subtitleFont);
        m_subtitleFont = nullptr;
    }
    
    TTF_Quit();
    
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
    // "ASO PLUS" 타이틀 표시
    if (m_titleTexture) {
        int textW, textH;
        SDL_QueryTexture(m_titleTexture, nullptr, nullptr, &textW, &textH);
        SDL_Rect titleRect = {
            (800 - textW) / 2,  // 중앙 정렬
            100,
            textW,
            textH
        };
        SDL_RenderCopy(m_renderer, m_titleTexture, nullptr, &titleRect);
    } else {
        // 폰트가 없을 경우 폴백: 간단한 박스
        SDL_SetRenderDrawColor(m_renderer, 255, 255, 255, 255);
        SDL_Rect titleRect = { 250, 150, 300, 50 };
        SDL_RenderDrawRect(m_renderer, &titleRect);
        
        for (int i = 0; i < 8; i++) {
            SDL_Rect letterRect = { 280 + i * 50, 160, 30, 30 };
            SDL_RenderFillRect(m_renderer, &letterRect);
        }
    }
    
    // "Press any Key To Start" 부제목 표시
    if (m_subtitleFont) {
        SDL_Color subtitleColor = {0, 255, 0, 255};  // 녹색
        SDL_Surface* subtitleSurface = TTF_RenderText_Blended(m_subtitleFont, "Press any Key To Start", subtitleColor);
        if (subtitleSurface) {
            SDL_Texture* subtitleTexture = SDL_CreateTextureFromSurface(m_renderer, subtitleSurface);
            if (subtitleTexture) {
                SDL_Rect subtitleRect = {
                    (800 - subtitleSurface->w) / 2,
                    300,
                    subtitleSurface->w,
                    subtitleSurface->h
                };
                SDL_RenderCopy(m_renderer, subtitleTexture, nullptr, &subtitleRect);
                SDL_DestroyTexture(subtitleTexture);
            }
            SDL_FreeSurface(subtitleSurface);
        }
    } else {
        // 폰트가 없을 경우 폴백
        SDL_SetRenderDrawColor(m_renderer, 0, 255, 0, 255);
        SDL_Rect startRect = { 300, 300, 200, 30 };
        SDL_RenderFillRect(m_renderer, &startRect);
    }
}

void Game::renderCountdown() {
    if (!m_titleFont) return;
    
    // 카운트다운 숫자 표시
    int countdown = static_cast<int>(m_stateTimer) + 1;
    SDL_Color countdownColor = {255, 255, 0, 255};  // 노란색
    std::string countdownText = std::to_string(countdown);
    SDL_Surface* countdownSurface = TTF_RenderText_Blended(m_titleFont, countdownText.c_str(), countdownColor);
    if (countdownSurface) {
        SDL_Texture* countdownTexture = SDL_CreateTextureFromSurface(m_renderer, countdownSurface);
        if (countdownTexture) {
            SDL_Rect countdownRect = {
                (800 - countdownSurface->w) / 2,
                250,
                countdownSurface->w,
                countdownSurface->h
            };
            SDL_RenderCopy(m_renderer, countdownTexture, nullptr, &countdownRect);
            SDL_DestroyTexture(countdownTexture);
        }
        SDL_FreeSurface(countdownSurface);
    }
    
    // "GET READY" 메시지
    if (m_uiFont) {
        SDL_Color readyColor = {255, 255, 255, 255};  // 흰색
        SDL_Surface* readySurface = TTF_RenderText_Blended(m_uiFont, "GET READY", readyColor);
        if (readySurface) {
            SDL_Texture* readyTexture = SDL_CreateTextureFromSurface(m_renderer, readySurface);
            if (readyTexture) {
                SDL_Rect readyRect = {
                    (800 - readySurface->w) / 2,
                    150,
                    readySurface->w,
                    readySurface->h
                };
                SDL_RenderCopy(m_renderer, readyTexture, nullptr, &readyRect);
                SDL_DestroyTexture(readyTexture);
            }
            SDL_FreeSurface(readySurface);
        }
    }
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
