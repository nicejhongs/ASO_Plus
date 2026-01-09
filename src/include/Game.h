#pragma once
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_image.h>
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
    
    SDL_Texture* m_spriteSheet;
    SDL_Rect m_playerSprite;
    SDL_Rect m_enemySprite;
    
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
