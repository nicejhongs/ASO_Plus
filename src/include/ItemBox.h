#pragma once
#include <SDL2/SDL.h>

class ItemBox {
public:
    enum class BoxState {
        HIDDEN,      // Box not opened (shows item_b.png)
        REVEALED_S,  // Speed revealed (shows item_s.png)
        REVEALED_L,  // Laser revealed (shows item_l.png)
        REVEALED_M   // Missile revealed (shows item_m.png)
    };

    ItemBox(float x, float y);
    ~ItemBox();

    void reveal();  // Called when hit by missile
    void update(float deltaTime);
    void render(SDL_Renderer* renderer, SDL_Texture* boxTexture, SDL_Texture* sTexture, SDL_Texture* lTexture, SDL_Texture* mTexture);

    float getX() const { return m_x; }
    float getY() const { return m_y; }
    float getWidth() const { return m_width; }
    float getHeight() const { return m_height; }
    BoxState getState() const { return m_state; }
    bool isRevealed() const { return m_state != BoxState::HIDDEN; }
    bool isOffScreen() const;

private:
    float m_x, m_y;
    float m_width, m_height;
    BoxState m_state;
    float m_blinkTimer;
};
