#include "Game.h"
#include <SDL2/SDL.h>

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;
const int FPS = 60;
const int FRAME_DELAY = 1000 / FPS;

int main(int argc, char* argv[]) {
    Game game;

    if (!game.init("2D Shooting Game", SCREEN_WIDTH, SCREEN_HEIGHT)) {
        SDL_Log("게임 초기화 실패!");
        return -1;
    }

    Uint32 frameStart;
    int frameTime;
    float deltaTime = 0.016f; // 초기값 (약 60 FPS)

    // 게임 루프
    while (game.isRunning()) {
        frameStart = SDL_GetTicks();

        game.handleEvents();
        game.update(deltaTime);
        game.render();

        frameTime = SDL_GetTicks() - frameStart;

        // 프레임 제한
        if (frameTime < FRAME_DELAY) {
            SDL_Delay(FRAME_DELAY - frameTime);
            frameTime = FRAME_DELAY;
        }

        // deltaTime 계산 (초 단위)
        deltaTime = frameTime / 1000.0f;
    }

    game.clean();

    return 0;
}
