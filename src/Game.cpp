#include "Game.h"
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <fstream>
#include <algorithm>

#ifdef _WIN32
#include <windows.h>
#endif

// Generate resource path based on executable file path
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
    , m_gameState(GameState::START_SCREEN)
    , m_stateTimer(0.0f)
    , m_lives(3)
    , m_score(0)
    , m_highScore(0)
    , m_shootSound(nullptr)
    , m_explosionSound(nullptr)
    , m_bgMusic(nullptr)
    , m_backgroundTexture(nullptr)
    , m_backgroundY1(0.0f)
    , m_backgroundY2(-960.0f)
    , m_backgroundScrollSpeed(50.0f)
    , m_playerTexture(nullptr)
    , m_enemyTexture(nullptr)
    , m_titleFont(nullptr)
    , m_uiFont(nullptr)
    , m_subtitleFont(nullptr)
    , m_titleTexture(nullptr)
    , m_startLogoTexture(nullptr)
    , m_blinkTimer(0.0f)
    , m_fadeAlpha(0)
{
    loadHighScores();
}

Game::~Game() {
    clean();
}

bool Game::init(const char* title, int width, int height) {
    // SDL initialization
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        SDL_Log("Failed to initialize SDL: %s", SDL_GetError());
        return false;
    }

    // Create window
    m_window = SDL_CreateWindow(
        title,
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        width, height,
        SDL_WINDOW_SHOWN
    );

    if (!m_window) {
        SDL_Log("Failed to create window: %s", SDL_GetError());
        return false;
    }
    
    // Update background positions based on actual window height
    m_backgroundY2 = -static_cast<float>(height);

    // Create renderer
    m_renderer = SDL_CreateRenderer(m_window, -1, SDL_RENDERER_ACCELERATED);
    if (!m_renderer) {
        SDL_Log("Failed to create renderer: %s", SDL_GetError());
        return false;
    }
    
    // Initialize SDL_ttf
    if (TTF_Init() == -1) {
        SDL_Log("Failed to initialize TTF: %s", TTF_GetError());
        // Continue without fonts
    }

    // Initialize random seed
    srand(static_cast<unsigned>(time(nullptr)));

    // Initialize SDL_mixer
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        SDL_Log("Failed to initialize SDL_mixer: %s", Mix_GetError());
        // Continue without sound
    }
    
    // Load sound files (WAV format, Mix_Chunk)
    std::string shootPath = getResourcePath("shot_01.wav");
    m_shootSound = Mix_LoadWAV(shootPath.c_str());
    if (!m_shootSound) {
        SDL_Log("INFO: Failed to load shot_01.wav: %s (path: %s)", Mix_GetError(), shootPath.c_str());
    } else {
        SDL_Log("INFO: Loaded shot_01.wav: %s", shootPath.c_str());
    }
    
    std::string explosionPath = getResourcePath("exlposion_01.wav");
    m_explosionSound = Mix_LoadWAV(explosionPath.c_str());
    if (!m_explosionSound) {
        SDL_Log("INFO: Failed to load exlposion_01.wav: %s (path: %s)", Mix_GetError(), explosionPath.c_str());
    } else {
        SDL_Log("INFO: Loaded exlposion_01.wav: %s", explosionPath.c_str());
    }
    
    // Load background music (WAV format, Mix_Music)
    std::string bgMusicPath = getResourcePath("aso_plus_opening.wav");
    m_bgMusic = Mix_LoadMUS(bgMusicPath.c_str());
    if (!m_bgMusic) {
        SDL_Log("INFO: Failed to load aso_plus_opening.wav: %s (path: %s)", Mix_GetError(), bgMusicPath.c_str());
    } else {
        SDL_Log("INFO: Loaded aso_plus_opening.wav: %s", bgMusicPath.c_str());
        // Play background music infinitely (-1 = loop)
        Mix_PlayMusic(m_bgMusic, -1);
        Mix_VolumeMusic(64); // Volume 50% (0-128)
    }

    // Load sprite images (PNG with transparency support)
    int imgFlags = IMG_INIT_PNG;
    if (!(IMG_Init(imgFlags) & imgFlags)) {
        SDL_Log("Failed to initialize SDL_image: %s", IMG_GetError());
    }
    
    // Load background image (background.png)
    std::string bgPath = getResourcePath("background.png");
    SDL_Surface* bgSurface = IMG_Load(bgPath.c_str());
    if (bgSurface) {
        m_backgroundTexture = SDL_CreateTextureFromSurface(m_renderer, bgSurface);
        SDL_FreeSurface(bgSurface);
        
        if (m_backgroundTexture) {
            SDL_Log("INFO: Loaded background.png: %s", bgPath.c_str());
        }
    } else {
        SDL_Log("INFO: Failed to load background.png: %s (path: %s)", IMG_GetError(), bgPath.c_str());
    }
    
    // Load player sprite (ship_01.png)
    std::string playerSpritePath = getResourcePath("ship_01.png");
    SDL_Surface* playerSurface = IMG_Load(playerSpritePath.c_str());
    if (playerSurface) {
        // Set white background as transparent (RGB: 255, 255, 255)
        SDL_SetColorKey(playerSurface, SDL_TRUE, SDL_MapRGB(playerSurface->format, 255, 255, 255));
        
        m_playerTexture = SDL_CreateTextureFromSurface(m_renderer, playerSurface);
        SDL_FreeSurface(playerSurface);
        
        if (m_playerTexture) {
            SDL_Log("INFO: Loaded ship_01.png: %s", playerSpritePath.c_str());
            SDL_SetTextureBlendMode(m_playerTexture, SDL_BLENDMODE_BLEND);
        }
    } else {
        SDL_Log("INFO: Failed to load ship_01.png: %s (path: %s)", IMG_GetError(), playerSpritePath.c_str());
    }
    
    // Load enemy sprite (enemy_02.png)
    std::string enemySpritePath = getResourcePath("enemy_02.png");
    SDL_Surface* enemySurface = IMG_Load(enemySpritePath.c_str());
    if (enemySurface) {
        // Set white background as transparent (RGB: 255, 255, 255)
        SDL_SetColorKey(enemySurface, SDL_TRUE, SDL_MapRGB(enemySurface->format, 255, 255, 255));
        
        m_enemyTexture = SDL_CreateTextureFromSurface(m_renderer, enemySurface);
        SDL_FreeSurface(enemySurface);
        
        if (m_enemyTexture) {
            SDL_Log("INFO: Loaded enemy_02.png: %s", enemySpritePath.c_str());
            SDL_SetTextureBlendMode(m_enemyTexture, SDL_BLENDMODE_BLEND);
        }
    } else {
        SDL_Log("INFO: Failed to load enemy_02.png: %s (path: %s)", IMG_GetError(), enemySpritePath.c_str());
    }
    
    // Load font (Windows default font)
    m_titleFont = TTF_OpenFont("C:\\Windows\\Fonts\\arial.ttf", 72);
    if (!m_titleFont) {
        SDL_Log("INFO: Failed to load font: %s", TTF_GetError());
    } else {
        // Render "ASO PLUS" text
        SDL_Color titleColor = {255, 255, 255, 255};  // White
        SDL_Surface* titleSurface = TTF_RenderText_Blended(m_titleFont, "ASO PLUS", titleColor);
        if (titleSurface) {
            m_titleTexture = SDL_CreateTextureFromSurface(m_renderer, titleSurface);
            SDL_FreeSurface(titleSurface);
            if (m_titleTexture) {
                SDL_Log("INFO: Created title text");
            }
        }
    }
    
    // Load UI font (for score, countdown)
    m_uiFont = TTF_OpenFont("C:\\Windows\\Fonts\\arial.ttf", 48);
    if (!m_uiFont) {
        SDL_Log("INFO: Failed to load UI font: %s", TTF_GetError());
    }
    
    // Load subtitle font
    m_subtitleFont = TTF_OpenFont("C:\\Windows\\Fonts\\arial.ttf", 32);
    if (!m_subtitleFont) {
        SDL_Log("INFO: Failed to load subtitle font: %s", TTF_GetError());
    }
    
    // Load start logo image (start_logo.png)
    std::string startLogoPath = getResourcePath("start_logo.png");
    SDL_Surface* startLogoSurface = IMG_Load(startLogoPath.c_str());
    if (startLogoSurface) {
        m_startLogoTexture = SDL_CreateTextureFromSurface(m_renderer, startLogoSurface);
        SDL_FreeSurface(startLogoSurface);
        
        if (m_startLogoTexture) {
            SDL_Log("INFO: Loaded start_logo.png: %s", startLogoPath.c_str());
            SDL_SetTextureBlendMode(m_startLogoTexture, SDL_BLENDMODE_BLEND);
        }
    } else {
        SDL_Log("INFO: Failed to load start_logo.png: %s (path: %s)", IMG_GetError(), startLogoPath.c_str());
    }

    // Create player (centered at bottom of screen)
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
            // Any key to start from START_SCREEN - transition to MENU with fade
            else if (m_gameState == GameState::START_SCREEN) {
                m_gameState = GameState::MENU;
                m_fadeAlpha = 0;
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
                // Click on START_SCREEN to transition to MENU
                if (m_gameState == GameState::START_SCREEN) {
                    m_gameState = GameState::MENU;
                    m_fadeAlpha = 0;
                }
                // Click on MENU or GAME_OVER to start countdown
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
                // PLAYING 상태에서는 총알 발사
                else if (m_gameState == GameState::PLAYING && m_player->canShoot()) {
                    int missileCount = m_player->getMissileCount();
                    float playerCenterX = m_player->getX() + m_player->getWidth() / 2;
                    
                    // Fire multiple bullets based on upgrade level
                    if (missileCount == 1) {
                        m_bullets.push_back(std::make_unique<Bullet>(
                            playerCenterX, m_player->getY(), Bullet::Owner::PLAYER
                        ));
                    } else {
                        float spacing = 15.0f;
                        float startX = playerCenterX - (missileCount - 1) * spacing / 2;
                        for (int i = 0; i < missileCount; i++) {
                            m_bullets.push_back(std::make_unique<Bullet>(
                                startX + i * spacing, m_player->getY(), Bullet::Owner::PLAYER
                            ));
                        }
                    }
                    
                    m_player->resetShootTimer();
                    
                    // Play shoot sound
                    if (m_shootSound) {
                        Mix_PlayChannel(-1, m_shootSound, 0);
                    } else {
                        // Fallback: Windows beep sound
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
    if (m_gameState == GameState::START_SCREEN) {
        // 깜빡임 타이머 업데이트
        m_blinkTimer += deltaTime;
        return;
    }
    else if (m_gameState == GameState::MENU) {
        // 페이드 인 효과
        if (m_fadeAlpha < 255) {
            float newAlpha = m_fadeAlpha + deltaTime * 300.0f;
            m_fadeAlpha = static_cast<Uint8>(newAlpha > 255.0f ? 255 : newAlpha);
        }
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
        // Do nothing in game over state
        return;
    }
    
    // Execute game logic only in PLAYING state
    // Background scroll
    m_backgroundY1 += m_backgroundScrollSpeed * deltaTime;
    m_backgroundY2 += m_backgroundScrollSpeed * deltaTime;
    int windowHeight;
    SDL_GetWindowSize(m_window, nullptr, &windowHeight);
    
    m_backgroundY1 += m_backgroundScrollSpeed * deltaTime;
    m_backgroundY2 += m_backgroundScrollSpeed * deltaTime;
    
    // Reset background position when it goes off screen
    if (m_backgroundY1 >= windowHeight) {
        m_backgroundY1 = m_backgroundY2 - windowHeight;
    }
    if (m_backgroundY2 >= windowHeight) {
        m_backgroundY2 = m_backgroundY1 - windowHeight;
    }
    
    // Update player
    m_player->update(deltaTime);
    
    // Check player screen boundaries
    int windowWidth;
    SDL_GetWindowSize(m_window, &windowWidth, &windowHeight);
    m_player->clampToScreen(windowWidth, windowHeight);

    // Spawn enemies
    m_enemySpawnTimer += deltaTime;
    if (m_enemySpawnTimer >= m_enemySpawnInterval) {
        int windowWidth, windowHeight;
        SDL_GetWindowSize(m_window, &windowWidth, &windowHeight);
        
        float enemyX = static_cast<float>(rand() % (windowWidth - 40));
        // 10% chance to spawn special enemy
        bool isSpecial = (rand() % 10) == 0;
        m_enemies.push_back(std::make_unique<Enemy>(enemyX, -40.0f, isSpecial));
        m_enemySpawnTimer = 0.0f;
    }

    // Update enemies
    for (auto& enemy : m_enemies) {
        enemy->update(deltaTime);
    }

    // Update bullets
    for (auto& bullet : m_bullets) {
        bullet->update(deltaTime);
    }

    // Remove enemies that went off screen
    m_enemies.erase(
        std::remove_if(m_enemies.begin(), m_enemies.end(),
            [](const std::unique_ptr<Enemy>& enemy) {
                return enemy->isOffScreen();
            }),
        m_enemies.end()
    );

    // Remove bullets that went off screen
    m_bullets.erase(
        std::remove_if(m_bullets.begin(), m_bullets.end(),
            [](const std::unique_ptr<Bullet>& bullet) {
                return bullet->isOffScreen();
            }),
        m_bullets.end()
    );

    // Collision detection (simple AABB)
    for (auto bulletIt = m_bullets.begin(); bulletIt != m_bullets.end();) {
        bool bulletRemoved = false;
        
        if ((*bulletIt)->getOwner() == Bullet::Owner::PLAYER) {
            for (auto enemyIt = m_enemies.begin(); enemyIt != m_enemies.end();) {
                // Check collision
                if ((*bulletIt)->getX() < (*enemyIt)->getX() + (*enemyIt)->getWidth() &&
                    (*bulletIt)->getX() + (*bulletIt)->getWidth() > (*enemyIt)->getX() &&
                    (*bulletIt)->getY() < (*enemyIt)->getY() + (*enemyIt)->getHeight() &&
                    (*bulletIt)->getY() + (*bulletIt)->getHeight() > (*enemyIt)->getY()) {
                    
                    // Check if special enemy - drop power-up
                    bool wasSpecial = (*enemyIt)->isSpecial();
                    float enemyX = (*enemyIt)->getX();
                    float enemyY = (*enemyIt)->getY();
                    
                    // Collision occurred: remove both and increase score
                    m_score += wasSpecial ? 50 : 10;
                    if (m_score > m_highScore) {
                        m_highScore = m_score;
                    }
                    
                    // Play explosion sound
                    if (m_explosionSound) {
                        Mix_PlayChannel(-1, m_explosionSound, 0);
                    }
                    
                    // Drop power-up if special
                    if (wasSpecial) {
                        dropPowerUp(enemyX + (*enemyIt)->getWidth() / 2, enemyY);
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
    
    // 파워업 업데이트
    for (auto& powerUp : m_powerUps) {
        powerUp->update(deltaTime);
    }
    
    // 화면 밖으로 나간 파워업 제거
    m_powerUps.erase(
        std::remove_if(m_powerUps.begin(), m_powerUps.end(),
            [](const std::unique_ptr<PowerUp>& powerUp) {
                return powerUp->isOffScreen();
            }),
        m_powerUps.end()
    );
    
    // 플레이어와 적 충돌 체크
    checkPlayerEnemyCollision();
    
    // 파워업 수집 체크
    checkPowerUpCollection();
}

void Game::checkPlayerEnemyCollision() {
    for (auto enemyIt = m_enemies.begin(); enemyIt != m_enemies.end();) {
        // Use hitbox instead of full sprite for more accurate collision
        float playerLeft = m_player->getHitboxX();
        float playerRight = playerLeft + m_player->getHitboxWidth();
        float playerTop = m_player->getHitboxY();
        float playerBottom = playerTop + m_player->getHitboxHeight();
        
        float enemyLeft = (*enemyIt)->getHitboxX();
        float enemyRight = enemyLeft + (*enemyIt)->getHitboxWidth();
        float enemyTop = (*enemyIt)->getHitboxY();
        float enemyBottom = enemyTop + (*enemyIt)->getHitboxHeight();
        
        // Check collision between hitboxes
        if (playerLeft < enemyRight &&
            playerRight > enemyLeft &&
            playerTop < enemyBottom &&
            playerBottom > enemyTop) {
            
            // Collision occurred: remove enemy, decrease life
            enemyIt = m_enemies.erase(enemyIt);
            m_lives--;
            
            // Play explosion sound
            if (m_explosionSound) {
                Mix_PlayChannel(-1, m_explosionSound, 0);
            } else {
                // Fallback: Windows beep sound
                #ifdef _WIN32
                Beep(300, 200);
                #endif
            }
            
            // If lives reach 0, game over
            if (m_lives <= 0) {
                // Flash screen red (explosion effect)
                SDL_SetRenderDrawColor(m_renderer, 255, 0, 0, 255);
                SDL_RenderClear(m_renderer);
                SDL_RenderPresent(m_renderer);
                SDL_Delay(300); // Wait 0.3 seconds
                
                // Clear screen to black
                SDL_SetRenderDrawColor(m_renderer, 0, 0, 0, 255);
                SDL_RenderClear(m_renderer);
                SDL_RenderPresent(m_renderer);
                SDL_Delay(200); // Wait 0.2 seconds
                
                // Save high score
                addHighScore(m_score);
                
                // Transition to game over state
                m_gameState = GameState::GAME_OVER;
                m_enemies.clear();
                m_bullets.clear();
            }
            else {
                // If lives remain, restart after countdown
                m_gameState = GameState::COUNTDOWN;
                m_stateTimer = 3.0f; // 3 second countdown
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
    // Clear screen (black)
    SDL_SetRenderDrawColor(m_renderer, 0, 0, 0, 255);
    SDL_RenderClear(m_renderer);
    
    // Draw background (first)
    if (m_backgroundTexture) {
        int windowWidth, windowHeight;
        SDL_GetWindowSize(m_window, &windowWidth, &windowHeight);
        SDL_Rect bg1 = {0, static_cast<int>(m_backgroundY1), windowWidth, windowHeight};
        SDL_Rect bg2 = {0, static_cast<int>(m_backgroundY2), windowWidth, windowHeight};
        SDL_RenderCopy(m_renderer, m_backgroundTexture, nullptr, &bg1);
        SDL_RenderCopy(m_renderer, m_backgroundTexture, nullptr, &bg2);
    }

    // Render based on game state
    if (m_gameState == GameState::START_SCREEN) {
        renderStartScreen();
    }
    else if (m_gameState == GameState::MENU) {
        renderMenu();
    }
    else if (m_gameState == GameState::COUNTDOWN) {
        renderCountdown();
    }
    else if (m_gameState == GameState::PLAYING) {
        // Render player (using single texture)
        if (m_playerTexture) {
            m_player->render(m_renderer, m_playerTexture, nullptr);
        } else {
            m_player->render(m_renderer);
        }

        // Render enemies (using single texture)
        for (auto& enemy : m_enemies) {
            if (m_enemyTexture) {
                enemy->render(m_renderer, m_enemyTexture, nullptr);
            } else {
                enemy->render(m_renderer);
            }
        }

        // Render bullets
        for (auto& bullet : m_bullets) {
            bullet->render(m_renderer);
        }
        
        // Render power-ups
        for (auto& powerUp : m_powerUps) {
            powerUp->render(m_renderer);
        }
        
        // Render UI
        renderUI();
    }
    else if (m_gameState == GameState::GAME_OVER) {
        renderGameOver();
    }

    // Present to screen
    SDL_RenderPresent(m_renderer);
}

void Game::renderUI() {
    if (!m_uiFont) return;
    
    // Top left: Display score
    SDL_Color scoreColor = {255, 255, 255, 255};  // White
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
    
    // Top center: Display high score
    SDL_Color highScoreColor = {255, 215, 0, 255}; // Gold
    std::string highScoreText = "HI: " + std::to_string(m_highScore);
    SDL_Surface* highScoreSurface = TTF_RenderText_Blended(m_uiFont, highScoreText.c_str(), highScoreColor);
    if (highScoreSurface) {
        SDL_Texture* highScoreTexture = SDL_CreateTextureFromSurface(m_renderer, highScoreSurface);
        if (highScoreTexture) {
            int windowWidth;
            SDL_GetWindowSize(m_window, &windowWidth, nullptr);
            SDL_Rect highScoreRect = {(windowWidth - highScoreSurface->w) / 2, 10, highScoreSurface->w, highScoreSurface->h};
            SDL_RenderCopy(m_renderer, highScoreTexture, nullptr, &highScoreRect);
            SDL_DestroyTexture(highScoreTexture);
        }
        SDL_FreeSurface(highScoreSurface);
    }
    
    // Top right: Display remaining lives
    SDL_Color livesColor = {0, 150, 255, 255};
    std::string livesText = "LIVES: " + std::to_string(m_lives);
    SDL_Surface* livesSurface = TTF_RenderText_Blended(m_uiFont, livesText.c_str(), livesColor);
    if (livesSurface) {
        SDL_Texture* livesTexture = SDL_CreateTextureFromSurface(m_renderer, livesSurface);
        if (livesTexture) {
            int windowWidth;
            SDL_GetWindowSize(m_window, &windowWidth, nullptr);
            SDL_Rect livesRect = {windowWidth - livesSurface->w - 10, 10, livesSurface->w, livesSurface->h};
            SDL_RenderCopy(m_renderer, livesTexture, nullptr, &livesRect);
            SDL_DestroyTexture(livesTexture);
        }
        SDL_FreeSurface(livesSurface);
    }
}

void Game::clean() {
    // Release sounds
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
    
    // Release textures
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
    if (m_startLogoTexture) {
        SDL_DestroyTexture(m_startLogoTexture);
        m_startLogoTexture = nullptr;
    }
    
    // Release fonts
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

void Game::renderStartScreen() {
    int windowWidth, windowHeight;
    SDL_GetWindowSize(m_window, &windowWidth, &windowHeight);
    
    // Black background
    SDL_SetRenderDrawColor(m_renderer, 0, 0, 0, 255);
    SDL_RenderClear(m_renderer);
    
    // Display start_logo.png (fullscreen or centered)
    if (m_startLogoTexture) {
        int logoW, logoH;
        SDL_QueryTexture(m_startLogoTexture, nullptr, nullptr, &logoW, &logoH);
        
        // Fit to fullscreen
        SDL_Rect logoRect = {0, 0, windowWidth, windowHeight};
        SDL_RenderCopy(m_renderer, m_startLogoTexture, nullptr, &logoRect);
    }
    
    // "Press Button to start" 깜빡이는 텍스트
    if (m_subtitleFont) {
        // 0.5초마다 깜빡임
        bool visible = (static_cast<int>(m_blinkTimer * 2) % 2) == 0;
        
        if (visible) {
            SDL_Color textColor = {255, 255, 255, 255};  // 흰색
            SDL_Surface* textSurface = TTF_RenderText_Blended(m_subtitleFont, "Press Button to start", textColor);
            if (textSurface) {
                SDL_Texture* textTexture = SDL_CreateTextureFromSurface(m_renderer, textSurface);
                if (textTexture) {
                    SDL_Rect textRect = {
                        (windowWidth - textSurface->w) / 2,
                        windowHeight - 100,  // Bottom of screen
                        textSurface->w,
                        textSurface->h
                    };
                    SDL_RenderCopy(m_renderer, textTexture, nullptr, &textRect);
                    SDL_DestroyTexture(textTexture);
                }
                SDL_FreeSurface(textSurface);
            }
        }
    }
}

void Game::renderMenu() {
    // Apply alpha value for fade-in effect
    // Display "ASO PLUS" title
    if (m_titleTexture) {
        int textW, textH, windowWidth;
        SDL_QueryTexture(m_titleTexture, nullptr, nullptr, &textW, &textH);
        SDL_GetWindowSize(m_window, &windowWidth, nullptr);
        SDL_SetTextureAlphaMod(m_titleTexture, m_fadeAlpha);
        SDL_Rect titleRect = {
            (windowWidth - textW) / 2,
            100,
            textW,
            textH
        };
        SDL_RenderCopy(m_renderer, m_titleTexture, nullptr, &titleRect);
    } else {
        // 폰트가 없을 경우 폴백: 간단한 박스
        int windowWidth;
        SDL_GetWindowSize(m_window, &windowWidth, nullptr);
        SDL_SetRenderDrawColor(m_renderer, 255, 255, 255, 255);
        SDL_Rect titleRect = { windowWidth / 2 - 150, 150, 300, 50 };
        SDL_RenderDrawRect(m_renderer, &titleRect);
        
        for (int i = 0; i < 8; i++) {
            SDL_Rect letterRect = { windowWidth / 2 - 120 + i * 50, 160, 30, 30 };
            SDL_RenderFillRect(m_renderer, &letterRect);
        }
    }
    
    // Display "Press any Key To Start" subtitle
    if (m_subtitleFont) {
        SDL_Color subtitleColor = {0, 255, 0, 255};  // Green
        SDL_Surface* subtitleSurface = TTF_RenderText_Blended(m_subtitleFont, "Press any Key To Start", subtitleColor);
        if (subtitleSurface) {
            SDL_Texture* subtitleTexture = SDL_CreateTextureFromSurface(m_renderer, subtitleSurface);
            if (subtitleTexture) {
                SDL_SetTextureAlphaMod(subtitleTexture, m_fadeAlpha);
                int windowWidth;
                SDL_GetWindowSize(m_window, &windowWidth, nullptr);
                SDL_Rect subtitleRect = {
                    (windowWidth - subtitleSurface->w) / 2,
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
        // Fallback when font is not available
        int windowWidth;
        SDL_GetWindowSize(m_window, &windowWidth, nullptr);
        SDL_SetRenderDrawColor(m_renderer, 0, 255, 0, 255);
        SDL_Rect startRect = { windowWidth / 2 - 100, 300, 200, 30 };
        SDL_RenderFillRect(m_renderer, &startRect);
    }
}

void Game::renderCountdown() {
    if (!m_titleFont) return;
    
    int windowWidth;
    SDL_GetWindowSize(m_window, &windowWidth, nullptr);
    
    // 카운트다운 숫자 표시
    int countdown = static_cast<int>(m_stateTimer) + 1;
    SDL_Color countdownColor = {255, 255, 0, 255};  // 노란색
    std::string countdownText = std::to_string(countdown);
    SDL_Surface* countdownSurface = TTF_RenderText_Blended(m_titleFont, countdownText.c_str(), countdownColor);
    if (countdownSurface) {
        SDL_Texture* countdownTexture = SDL_CreateTextureFromSurface(m_renderer, countdownSurface);
        if (countdownTexture) {
            SDL_Rect countdownRect = {
                (windowWidth - countdownSurface->w) / 2,
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
                    (windowWidth - readySurface->w) / 2,
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

void Game::dropPowerUp(float x, float y) {
    // Random power-up type
    int type = rand() % 3;
    PowerUpType powerUpType;
    
    switch (type) {
        case 0:
            powerUpType = PowerUpType::SPEED;
            break;
        case 1:
            powerUpType = PowerUpType::MISSILE;
            break;
        case 2:
            powerUpType = PowerUpType::ONE_UP;
            break;
    }
    
    m_powerUps.push_back(std::make_unique<PowerUp>(x - 15, y, powerUpType));
}

void Game::checkPowerUpCollection() {
    for (auto powerUpIt = m_powerUps.begin(); powerUpIt != m_powerUps.end();) {
        // Check collision with player
        if (m_player->getX() < (*powerUpIt)->getX() + (*powerUpIt)->getWidth() &&
            m_player->getX() + m_player->getWidth() > (*powerUpIt)->getX() &&
            m_player->getY() < (*powerUpIt)->getY() + (*powerUpIt)->getHeight() &&
            m_player->getY() + m_player->getHeight() > (*powerUpIt)->getY()) {
            
            // Apply power-up effect
            switch ((*powerUpIt)->getType()) {
                case PowerUpType::SPEED:
                    m_player->upgradeSpeed();
                    SDL_Log("INFO: Speed upgraded!");
                    break;
                case PowerUpType::MISSILE:
                    m_player->upgradeMissile();
                    SDL_Log("INFO: Missile upgraded! Count: %d", m_player->getMissileCount());
                    break;
                case PowerUpType::ONE_UP:
                    m_lives++;
                    SDL_Log("INFO: 1UP! Lives: %d", m_lives);
                    break;
            }
            
            // Play sound (reuse shoot sound for now)
            if (m_shootSound) {
                Mix_PlayChannel(-1, m_shootSound, 0);
            }
            
            powerUpIt = m_powerUps.erase(powerUpIt);
        } else {
            ++powerUpIt;
        }
    }
}
