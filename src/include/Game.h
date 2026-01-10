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
#include "Bullet.h"
#include "PowerUp.h"

class Game {
public:
    enum class GameState {
        START_SCREEN,
        MENU,
        COUNTDOWN,
        PLAYING,
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

    std::unique_ptr<Player> m_player;
    std::vector<std::unique_ptr<Enemy>> m_enemies;
    std::vector<std::unique_ptr<Bullet>> m_bullets;
    std::vector<std::unique_ptr<Bullet>> m_enemyBullets;  // Enemy bullets
    std::vector<std::unique_ptr<PowerUp>> m_powerUps;

    float m_enemySpawnTimer;
    const float m_enemySpawnInterval = 2.0f;
    
    GameState m_gameState;
    float m_stateTimer;
    
    int m_lives;
    int m_score;
    int m_highScore;
    std::vector<int> m_highScores;
    
    Mix_Chunk* m_shootSound;
    Mix_Chunk* m_explosionSound;
    Mix_Music* m_bgMusic;  // Background music
    
    SDL_Texture* m_backgroundTexture;  // Background texture
    float m_backgroundY1;  // First background position
    float m_backgroundY2;  // Second background position
    float m_backgroundScrollSpeed;  // Scroll speed
    
    SDL_Texture* m_playerTexture;  // ship_01.png (legacy)
    SDL_Texture* m_shipStopTexture;  // ship_stop.png
    SDL_Texture* m_shipForwardTexture;  // ship_forward.png
    SDL_Texture* m_shipBackwardTexture;  // ship_backward.png
    SDL_Texture* m_shipLeftTexture;  // ship_left.png
    SDL_Texture* m_shipRightTexture;  // ship_right.png
    SDL_Texture* m_enemyTexture;   // enemy_02.png
    SDL_Texture* m_enemy03Texture; // enemy_03.png (special/item enemy)
    
    SDL_Texture* m_boom01Texture;  // boom01.png
    SDL_Texture* m_boom02Texture;  // boom02.png
    SDL_Texture* m_boom03Texture;  // boom03.png
    SDL_Texture* m_boom04Texture;  // boom04.png
    SDL_Texture* m_boom05Texture;  // boom05.png
    SDL_Texture* m_boom06Texture;  // boom06.png
    
    TTF_Font* m_titleFont;  // Title font
    TTF_Font* m_uiFont;     // UI font (score, countdown)
    TTF_Font* m_subtitleFont;  // Subtitle font
    SDL_Texture* m_titleTexture;  // Title texture
    
    SDL_Texture* m_startLogoTexture;  // Start logo texture
    float m_blinkTimer;  // Blink timer
    Uint8 m_fadeAlpha;  // Fade alpha value (0-255)
    
    void checkPlayerEnemyCollision();
    void checkPlayerEnemyBulletCollision();
    void checkPowerUpCollection();
    void dropPowerUp(float x, float y);
    void damagePlayer();
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
