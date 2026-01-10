#include "ItemBox.h"
#include <cstdlib>

ItemBox::ItemBox(float x, float y)
    : m_x(x)
    , m_y(y)
    , m_width(60.0f)  // Increased from 40 to 60
    , m_height(60.0f)  // Increased from 40 to 60
    , m_state(BoxState::HIDDEN)
    , m_blinkTimer(0.0f)
{
}

ItemBox::~ItemBox() {
}

void ItemBox::reveal() {
    if (m_state == BoxState::HIDDEN) {
        // Randomly choose S, L, or M when revealed
        int randomChoice = rand() % 3;
        switch (randomChoice) {
            case 0:
                m_state = BoxState::REVEALED_S;
                break;
            case 1:
                m_state = BoxState::REVEALED_L;
                break;
            case 2:
                m_state = BoxState::REVEALED_M;
                break;
        }
    }
}

void ItemBox::update(float deltaTime) {
    m_blinkTimer += deltaTime;
    
    // Move with background scroll speed (attached to background)
    m_y += 50.0f * deltaTime;  // Same as background scroll speed
}

void ItemBox::render(SDL_Renderer* renderer, SDL_Texture* boxTexture, SDL_Texture* sTexture, SDL_Texture* lTexture, SDL_Texture* mTexture) {
    SDL_Rect dstRect = {
        static_cast<int>(m_x),
        static_cast<int>(m_y),
        static_cast<int>(m_width),
        static_cast<int>(m_height)
    };

    SDL_Texture* currentTexture = nullptr;
    
    if (m_state == BoxState::HIDDEN) {
        currentTexture = boxTexture;  // Show box when hidden
    } else if (m_state == BoxState::REVEALED_S) {
        currentTexture = sTexture;  // Show S when revealed
    } else if (m_state == BoxState::REVEALED_L) {
        currentTexture = lTexture;  // Show L when revealed
    } else if (m_state == BoxState::REVEALED_M) {
        currentTexture = mTexture;  // Show M when revealed
    }

    if (currentTexture) {
        SDL_RenderCopy(renderer, currentTexture, nullptr, &dstRect);
    } else {
        // Fallback: draw colored rectangle
        if (m_state == BoxState::HIDDEN) {
            SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);  // Gray box
        } else if (m_state == BoxState::REVEALED_S) {
            SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);  // Green for speed
        } else if (m_state == BoxState::REVEALED_L) {
            SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);  // Yellow for laser
        } else if (m_state == BoxState::REVEALED_M) {
            SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);  // Red for missile
        }
        SDL_RenderFillRect(renderer, &dstRect);
        
        // Debug: log when falling back to colored rectangle
        static int logCount = 0;
        if (logCount < 5) {
            SDL_Log("WARNING: ItemBox using fallback rendering (texture is null)");
            logCount++;
        }
    }
}

bool ItemBox::isOffScreen() const {
    return m_y > 1200;  // Off bottom of screen
}
