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

class Game {
public:
    enum class GameState {
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

private:
    SDL_Window* m_window;
    SDL_Renderer* m_renderer;
    bool m_running;
    bool m_mouseGrabbed;  // Mouse lock state

    std::unique_ptr<Player> m_player;
    std::vector<std::unique_ptr<Enemy>> m_enemies;
    std::vector<std::unique_ptr<Bullet>> m_bullets;

    float m_enemySpawnTimer;
    const float m_enemySpawnInterval = 2.0f; // 2초마다 적 생성
    
    GameState m_gameState;
    float m_stateTimer;
    
    int m_lives;
    int m_score;
    int m_highScore;
    std::vector<int> m_highScores;
    
    Mix_Chunk* m_shootSound;
    Mix_Chunk* m_explosionSound;
    Mix_Music* m_bgMusic;  // 배경음악
    
    SDL_Texture* m_backgroundTexture;  // 배경화면
    float m_backgroundY1;  // 첫 번째 배경 위치
    float m_backgroundY2;  // 두 번째 배경 위치
    float m_backgroundScrollSpeed;  // 스크롤 속도
    
    SDL_Texture* m_playerTexture;  // ship_01.png
    SDL_Texture* m_enemyTexture;   // enemy_02.png
    
    TTF_Font* m_titleFont;  // 타이틀 폰트
    TTF_Font* m_uiFont;     // UI 폰트 (점수, 카운트다운)
    TTF_Font* m_subtitleFont;  // 부제목 폰트
    SDL_Texture* m_titleTexture;  // 타이틀 텍스처
    
    void checkPlayerEnemyCollision();
    void renderUI();
    void renderMenu();
    void renderCountdown();
    void renderGameOver();
    void drawNumber(int number, int x, int y, int scale);
    void saveHighScores();
    void loadHighScores();
    void addHighScore(int score);
};
