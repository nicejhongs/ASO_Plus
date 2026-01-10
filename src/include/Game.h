#pragma once
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <vector>
#include <memory>
#include <string>
#include "Player.h"
#include "Enemy.h"
#include "Boss.h"
#include "Bullet.h"
#include "PowerUp.h"
#include "ItemBox.h"

class Game {
public:
    enum class GameState {
        START_SCREEN,
        MENU,
        COUNTDOWN,
        PLAYING,
        PAUSED,
        GAME_OVER
    };

    Game();
    ~Game();

    bool init(const char* title, int width, int height);
    void handleEvents();
    void update(float deltaTime);
    void render();
    void clean();

    bool isRunning() const { return m_running; }
    int getWindowWidth() const { int w; SDL_GetWindowSize(m_window, &w, nullptr); return w; }
    int getWindowHeight() const { int h; SDL_GetWindowSize(m_window, nullptr, &h); return h; }

private:
    SDL_Window* m_window;
    SDL_Renderer* m_renderer;
    bool m_running;
    bool m_mouseGrabbed;  // Mouse lock state
    bool m_mousePressed;  // Mouse button pressed state
    float m_missileShootTimer;  // Missile shoot cooldown timer
    const float m_missileShootCooldown = 1.0f;  // Missile shoots every 1.0 second (2x slower than laser)

    std::unique_ptr<Player> m_player;
    std::vector<std::unique_ptr<Enemy>> m_enemies;
    std::unique_ptr<Boss> m_boss;  // Current boss
    std::vector<std::unique_ptr<Bullet>> m_bullets;
    std::vector<std::unique_ptr<Bullet>> m_enemyBullets;  // Enemy bullets
    std::vector<std::unique_ptr<PowerUp>> m_powerUps;
    std::vector<std::unique_ptr<ItemBox>> m_itemBoxes;  // Item boxes

    float m_enemySpawnTimer;
    const float m_enemySpawnInterval = 2.0f;
    int m_enemyKillCount;  // Count enemies killed to trigger boss
    int m_currentStage;    // Current stage number
    
    GameState m_gameState;
    float m_stateTimer;
    
    int m_lives;
    int m_score;
    int m_highScore;
    std::vector<int> m_highScores;
    
    Mix_Chunk* m_shootSound;
    Mix_Chunk* m_explosionSound;
    Mix_Music* m_bgMusic;  // Background music (default: aso_plus_opening2.wav)
    Mix_Music* m_stage01Music;  // Stage 1 music
    Mix_Music* m_stage02Music;  // Stage 2 music
    Mix_Music* m_boss01Music;  // Boss 1 music
    Mix_Music* m_boss02Music;  // Boss 2 music
    Mix_Music* m_boss03Music;  // Boss 3 music
    Mix_Music* m_boss04Music;  // Boss 4 music
    
    SDL_Texture* m_backgroundTexture;  // Background texture (legacy)
    SDL_Texture* m_background01Texture;  // Background 01
    SDL_Texture* m_background02Texture;  // Background 02
    SDL_Texture* m_background03Texture;  // Background 03
    SDL_Texture* m_background04Texture;  // Background 04
    
    // Background scrolling slots (two textures scrolling)
    SDL_Texture* m_bgSlot1Texture;  // Top background slot
    SDL_Texture* m_bgSlot2Texture;  // Bottom background slot
    float m_backgroundY1;  // First background position
    float m_backgroundY2;  // Second background position
    float m_backgroundScrollSpeed;  // Scroll speed
    
    // Background sequence tracking
    int m_bgSequenceIndex;  // Current position in sequence (0=01, 1=02, 2=03, 3=03repeat, 4=04, 5=01boss)
    int m_bg03LoopCount;  // How many times bg03 has looped
    int m_maxBg03Loops;  // Maximum loops for bg03 before switching to bg04
    SDL_Texture* m_currentSequenceTexture;  // Currently active texture in sequence
    SDL_Texture* m_nextSequenceTexture;  // Next texture to be loaded
    float m_bossSpawnTimer;  // Timer for boss spawn after bg04
    bool m_bg04Completed;  // Whether bg04 has been completed
    float m_itemBoxSpawnTimer;  // Timer for spawning item boxes in space city
    bool m_inSpaceCity;  // Whether currently in space city section (bg02-04)
    
    SDL_Texture* m_playerTexture;  // ship_01.png (legacy)
    SDL_Texture* m_shipStopTexture;  // ship_stop.png
    SDL_Texture* m_shipForwardTexture;  // ship_forward.png
    SDL_Texture* m_shipBackwardTexture;  // ship_backward.png
    SDL_Texture* m_shipLeftTexture;  // ship_left.png
    SDL_Texture* m_shipRightTexture;  // ship_right.png
    SDL_Texture* m_enemy01Texture; // enemy_01.png
    SDL_Texture* m_enemy02Texture; // enemy_02.png
    SDL_Texture* m_enemy03Texture; // enemy_03.png (special/item enemy)
    SDL_Texture* m_enemy04Texture; // enemy_04.png
    SDL_Texture* m_enemy05Texture; // enemy_05.png
    SDL_Texture* m_enemyTexture;   // enemy_02.png (legacy)
    SDL_Texture* m_boss01Texture;  // boss01.png
    
    SDL_Texture* m_boom01Texture;  // boom01.png
    SDL_Texture* m_boom02Texture;  // boom02.png
    SDL_Texture* m_boom03Texture;  // boom03.png
    SDL_Texture* m_boom04Texture;  // boom04.png
    SDL_Texture* m_boom05Texture;  // boom05.png
    SDL_Texture* m_boom06Texture;  // boom06.png
    
    SDL_Texture* m_itemBoxTexture;  // item_box.png (hidden box)
    SDL_Texture* m_itemSTexture;    // item_s.png (speed)
    SDL_Texture* m_itemLTexture;    // item_l.png (laser)
    SDL_Texture* m_itemMTexture;    // item_m.png (missile)
    
    TTF_Font* m_titleFont;  // Title font
    TTF_Font* m_uiFont;     // UI font (score, countdown)
    TTF_Font* m_subtitleFont;  // Subtitle font
    SDL_Texture* m_titleTexture;  // Title texture
    
    SDL_Texture* m_startLogoTexture;  // Start logo texture
    float m_blinkTimer;  // Blink timer
    Uint8 m_fadeAlpha;  // Fade alpha value (0-255)
    
    void checkPlayerEnemyCollision();
    void checkPlayerEnemyBulletCollision();
    void checkPlayerBossCollision();
    void checkBossBulletCollision();
    void checkPowerUpCollection();
    void checkItemBoxCollection();
    void checkMissileItemBoxCollision();
    void spawnItemBoxes();
    void dropPowerUp(float x, float y);
    void damagePlayer();
    void spawnBoss(int stage);
    void renderUI();
    void renderStartScreen();
    void renderMenu();
    void renderCountdown();
    void renderGameOver();
    void draw7SegmentDigit(int digit, int x, int y, int width, int height, SDL_Color color);
    void draw7SegmentNumber(int number, int x, int y, int digitWidth, int digitHeight, int spacing, SDL_Color color);
    void drawNumber(int number, int x, int y, int scale);
    void saveHighScores();
    void loadHighScores();
    void addHighScore(int score);
};
