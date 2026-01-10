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
    , m_stage01Music(nullptr)
    , m_stage02Music(nullptr)
    , m_boss01Music(nullptr)
    , m_boss02Music(nullptr)
    , m_boss03Music(nullptr)
    , m_boss04Music(nullptr)
    , m_backgroundTexture(nullptr)
    , m_background01Texture(nullptr)
    , m_background02Texture(nullptr)
    , m_background03Texture(nullptr)
    , m_background04Texture(nullptr)
    , m_bgSlot1Texture(nullptr)
    , m_bgSlot2Texture(nullptr)
    , m_backgroundY1(0.0f)
    , m_backgroundY2(-960.0f)
    , m_backgroundScrollSpeed(50.0f)
    , m_bgSequenceIndex(0)
    , m_bg03LoopCount(0)
    , m_maxBg03Loops(3)
    , m_currentSequenceTexture(nullptr)
    , m_nextSequenceTexture(nullptr)
    , m_bossSpawnTimer(0.0f)
    , m_bg04Completed(false)
    , m_itemBoxSpawnTimer(0.0f)
    , m_inSpaceCity(false)
    , m_playerTexture(nullptr)
    , m_shipStopTexture(nullptr)
    , m_shipForwardTexture(nullptr)
    , m_shipBackwardTexture(nullptr)
    , m_shipLeftTexture(nullptr)
    , m_shipRightTexture(nullptr)
    , m_enemy01Texture(nullptr)
    , m_enemy02Texture(nullptr)
    , m_enemy03Texture(nullptr)
    , m_enemy04Texture(nullptr)
    , m_enemy05Texture(nullptr)
    , m_enemyTexture(nullptr)
    , m_boss01Texture(nullptr)
    , m_boss(nullptr)
    , m_enemyKillCount(0)
    , m_currentStage(1)
    , m_boom01Texture(nullptr)
    , m_boom02Texture(nullptr)
    , m_boom03Texture(nullptr)
    , m_boom04Texture(nullptr)
    , m_boom05Texture(nullptr)
    , m_boom06Texture(nullptr)
    , m_itemBoxTexture(nullptr)
    , m_itemSTexture(nullptr)
    , m_itemLTexture(nullptr)
    , m_itemMTexture(nullptr)
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
    
    std::string explosionPath = getResourcePath("explosion_01.wav");
    m_explosionSound = Mix_LoadWAV(explosionPath.c_str());
    if (!m_explosionSound) {
        SDL_Log("INFO: Failed to load explosion_01.wav: %s (path: %s)", Mix_GetError(), explosionPath.c_str());
    } else {
        SDL_Log("INFO: Loaded explosion_01.wav: %s", explosionPath.c_str());
    }
    
    // Load background music (WAV format, Mix_Music)
    std::string bgMusicPath = getResourcePath("aso_plus_opening2.wav");
    m_bgMusic = Mix_LoadMUS(bgMusicPath.c_str());
    if (!m_bgMusic) {
        SDL_Log("INFO: Failed to load aso_plus_opening2.wav: %s (path: %s)", Mix_GetError(), bgMusicPath.c_str());
    } else {
        SDL_Log("INFO: Loaded aso_plus_opening2.wav: %s", bgMusicPath.c_str());
        // Play background music infinitely (-1 = loop)
        Mix_PlayMusic(m_bgMusic, -1);
        Mix_VolumeMusic(64); // Volume 50% (0-128)
    }
    
    // Load stage music
    std::string stage01Path = getResourcePath("stage01.wav");
    m_stage01Music = Mix_LoadMUS(stage01Path.c_str());
    if (m_stage01Music) {
        SDL_Log("INFO: Loaded stage01.wav: %s", stage01Path.c_str());
    } else {
        SDL_Log("INFO: Failed to load stage01.wav: %s", Mix_GetError());
    }
    
    // Load boss music
    std::string boss01MusicPath = getResourcePath("boss01.wav");
    m_boss01Music = Mix_LoadMUS(boss01MusicPath.c_str());
    if (m_boss01Music) {
        SDL_Log("INFO: Loaded boss01.wav: %s", boss01MusicPath.c_str());
    } else {
        SDL_Log("INFO: Failed to load boss01.wav: %s", Mix_GetError());
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
    
    // Load background textures (background01 ~ background04)
    std::string bg01Path = getResourcePath("background01.png");
    SDL_Surface* bg01Surface = IMG_Load(bg01Path.c_str());
    if (bg01Surface) {
        m_background01Texture = SDL_CreateTextureFromSurface(m_renderer, bg01Surface);
        SDL_FreeSurface(bg01Surface);
        if (m_background01Texture) {
            SDL_Log("INFO: Loaded background01.png: %s", bg01Path.c_str());
        }
    }
    
    std::string bg02Path = getResourcePath("background02.png");
    SDL_Surface* bg02Surface = IMG_Load(bg02Path.c_str());
    if (bg02Surface) {
        m_background02Texture = SDL_CreateTextureFromSurface(m_renderer, bg02Surface);
        SDL_FreeSurface(bg02Surface);
        if (m_background02Texture) {
            SDL_Log("INFO: Loaded background02.png: %s", bg02Path.c_str());
        }
    }
    
    std::string bg03Path = getResourcePath("background03.png");
    SDL_Surface* bg03Surface = IMG_Load(bg03Path.c_str());
    if (bg03Surface) {
        m_background03Texture = SDL_CreateTextureFromSurface(m_renderer, bg03Surface);
        SDL_FreeSurface(bg03Surface);
        if (m_background03Texture) {
            SDL_Log("INFO: Loaded background03.png: %s", bg03Path.c_str());
        }
    }
    
    std::string bg04Path = getResourcePath("background04.png");
    SDL_Surface* bg04Surface = IMG_Load(bg04Path.c_str());
    if (bg04Surface) {
        m_background04Texture = SDL_CreateTextureFromSurface(m_renderer, bg04Surface);
        SDL_FreeSurface(bg04Surface);
        if (m_background04Texture) {
            SDL_Log("INFO: Loaded background04.png: %s", bg04Path.c_str());
        }
    }
    
    // Initialize background scrolling slots
    // Both slots start with background01, next will be background02
    m_bgSlot1Texture = m_background01Texture;
    m_bgSlot2Texture = m_background01Texture;  // Same as slot1 initially
    m_bgSequenceIndex = 0;  // Starting with bg01
    m_currentSequenceTexture = m_background01Texture;
    m_nextSequenceTexture = m_background02Texture;  // 02 will come after 01
    
    // Load player sprite (ship_01.png) - legacy
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
    
    // Load ship_stop sprite
    std::string shipStopPath = getResourcePath("ship_stop.png");
    SDL_Surface* shipStopSurface = IMG_Load(shipStopPath.c_str());
    if (shipStopSurface) {
        SDL_SetColorKey(shipStopSurface, SDL_TRUE, SDL_MapRGB(shipStopSurface->format, 255, 255, 255));
        m_shipStopTexture = SDL_CreateTextureFromSurface(m_renderer, shipStopSurface);
        SDL_FreeSurface(shipStopSurface);
        if (m_shipStopTexture) {
            SDL_Log("INFO: Loaded ship_stop.png: %s", shipStopPath.c_str());
            SDL_SetTextureBlendMode(m_shipStopTexture, SDL_BLENDMODE_BLEND);
        }
    } else {
        SDL_Log("INFO: Failed to load ship_stop.png: %s (path: %s)", IMG_GetError(), shipStopPath.c_str());
    }
    
    // Load ship_forward sprite
    std::string shipForwardPath = getResourcePath("ship_forward.png");
    SDL_Surface* shipForwardSurface = IMG_Load(shipForwardPath.c_str());
    if (shipForwardSurface) {
        SDL_SetColorKey(shipForwardSurface, SDL_TRUE, SDL_MapRGB(shipForwardSurface->format, 255, 255, 255));
        m_shipForwardTexture = SDL_CreateTextureFromSurface(m_renderer, shipForwardSurface);
        SDL_FreeSurface(shipForwardSurface);
        if (m_shipForwardTexture) {
            SDL_Log("INFO: Loaded ship_forward.png: %s", shipForwardPath.c_str());
            SDL_SetTextureBlendMode(m_shipForwardTexture, SDL_BLENDMODE_BLEND);
        }
    } else {
        SDL_Log("INFO: Failed to load ship_forward.png: %s (path: %s)", IMG_GetError(), shipForwardPath.c_str());
    }
    
    // Load ship_backward sprite
    std::string shipBackwardPath = getResourcePath("ship_backward.png");
    SDL_Surface* shipBackwardSurface = IMG_Load(shipBackwardPath.c_str());
    if (shipBackwardSurface) {
        SDL_SetColorKey(shipBackwardSurface, SDL_TRUE, SDL_MapRGB(shipBackwardSurface->format, 255, 255, 255));
        m_shipBackwardTexture = SDL_CreateTextureFromSurface(m_renderer, shipBackwardSurface);
        SDL_FreeSurface(shipBackwardSurface);
        if (m_shipBackwardTexture) {
            SDL_Log("INFO: Loaded ship_backward.png: %s", shipBackwardPath.c_str());
            SDL_SetTextureBlendMode(m_shipBackwardTexture, SDL_BLENDMODE_BLEND);
        }
    } else {
        SDL_Log("INFO: Failed to load ship_backward.png: %s (path: %s)", IMG_GetError(), shipBackwardPath.c_str());
    }
    
    // Load ship_left sprite
    std::string shipLeftPath = getResourcePath("ship_left.png");
    SDL_Surface* shipLeftSurface = IMG_Load(shipLeftPath.c_str());
    if (shipLeftSurface) {
        SDL_SetColorKey(shipLeftSurface, SDL_TRUE, SDL_MapRGB(shipLeftSurface->format, 255, 255, 255));
        m_shipLeftTexture = SDL_CreateTextureFromSurface(m_renderer, shipLeftSurface);
        SDL_FreeSurface(shipLeftSurface);
        if (m_shipLeftTexture) {
            SDL_Log("INFO: Loaded ship_left.png: %s", shipLeftPath.c_str());
            SDL_SetTextureBlendMode(m_shipLeftTexture, SDL_BLENDMODE_BLEND);
        }
    } else {
        SDL_Log("INFO: Failed to load ship_left.png: %s (path: %s)", IMG_GetError(), shipLeftPath.c_str());
    }
    
    // Load ship_right sprite
    std::string shipRightPath = getResourcePath("ship_right.png");
    SDL_Surface* shipRightSurface = IMG_Load(shipRightPath.c_str());
    if (shipRightSurface) {
        SDL_SetColorKey(shipRightSurface, SDL_TRUE, SDL_MapRGB(shipRightSurface->format, 255, 255, 255));
        m_shipRightTexture = SDL_CreateTextureFromSurface(m_renderer, shipRightSurface);
        SDL_FreeSurface(shipRightSurface);
        if (m_shipRightTexture) {
            SDL_Log("INFO: Loaded ship_right.png: %s", shipRightPath.c_str());
            SDL_SetTextureBlendMode(m_shipRightTexture, SDL_BLENDMODE_BLEND);
        }
    } else {
        SDL_Log("INFO: Failed to load ship_right.png: %s (path: %s)", IMG_GetError(), shipRightPath.c_str());
    }
    
    // Load enemy sprites (enemy_01 ~ enemy_05)
    std::string enemy01Path = getResourcePath("enemy_01.png");
    SDL_Surface* enemy01Surface = IMG_Load(enemy01Path.c_str());
    if (enemy01Surface) {
        SDL_SetColorKey(enemy01Surface, SDL_TRUE, SDL_MapRGB(enemy01Surface->format, 255, 255, 255));
        m_enemy01Texture = SDL_CreateTextureFromSurface(m_renderer, enemy01Surface);
        SDL_FreeSurface(enemy01Surface);
        if (m_enemy01Texture) {
            SDL_Log("INFO: Loaded enemy_01.png: %s", enemy01Path.c_str());
            SDL_SetTextureBlendMode(m_enemy01Texture, SDL_BLENDMODE_BLEND);
        }
    } else {
        SDL_Log("INFO: Failed to load enemy_01.png: %s (path: %s)", IMG_GetError(), enemy01Path.c_str());
    }
    
    std::string enemy02Path = getResourcePath("enemy_02.png");
    SDL_Surface* enemy02Surface = IMG_Load(enemy02Path.c_str());
    if (enemy02Surface) {
        SDL_SetColorKey(enemy02Surface, SDL_TRUE, SDL_MapRGB(enemy02Surface->format, 255, 255, 255));
        m_enemy02Texture = SDL_CreateTextureFromSurface(m_renderer, enemy02Surface);
        SDL_FreeSurface(enemy02Surface);
        if (m_enemy02Texture) {
            SDL_Log("INFO: Loaded enemy_02.png: %s", enemy02Path.c_str());
            SDL_SetTextureBlendMode(m_enemy02Texture, SDL_BLENDMODE_BLEND);
        }
    } else {
        SDL_Log("INFO: Failed to load enemy_02.png: %s (path: %s)", IMG_GetError(), enemy02Path.c_str());
    }
    
    std::string enemy03Path = getResourcePath("enemy_03.png");
    SDL_Surface* enemy03Surface = IMG_Load(enemy03Path.c_str());
    if (enemy03Surface) {
        SDL_SetColorKey(enemy03Surface, SDL_TRUE, SDL_MapRGB(enemy03Surface->format, 255, 255, 255));
        m_enemy03Texture = SDL_CreateTextureFromSurface(m_renderer, enemy03Surface);
        SDL_FreeSurface(enemy03Surface);
        if (m_enemy03Texture) {
            SDL_Log("INFO: Loaded enemy_03.png: %s", enemy03Path.c_str());
            SDL_SetTextureBlendMode(m_enemy03Texture, SDL_BLENDMODE_BLEND);
        }
    } else {
        SDL_Log("INFO: Failed to load enemy_03.png: %s (path: %s)", IMG_GetError(), enemy03Path.c_str());
    }
    
    std::string enemy04Path = getResourcePath("enemy_04.png");
    SDL_Surface* enemy04Surface = IMG_Load(enemy04Path.c_str());
    if (enemy04Surface) {
        SDL_SetColorKey(enemy04Surface, SDL_TRUE, SDL_MapRGB(enemy04Surface->format, 255, 255, 255));
        m_enemy04Texture = SDL_CreateTextureFromSurface(m_renderer, enemy04Surface);
        SDL_FreeSurface(enemy04Surface);
        if (m_enemy04Texture) {
            SDL_Log("INFO: Loaded enemy_04.png: %s", enemy04Path.c_str());
            SDL_SetTextureBlendMode(m_enemy04Texture, SDL_BLENDMODE_BLEND);
        }
    } else {
        SDL_Log("INFO: Failed to load enemy_04.png: %s (path: %s)", IMG_GetError(), enemy04Path.c_str());
    }
    
    std::string enemy05Path = getResourcePath("enemy_05.png");
    SDL_Surface* enemy05Surface = IMG_Load(enemy05Path.c_str());
    if (enemy05Surface) {
        SDL_SetColorKey(enemy05Surface, SDL_TRUE, SDL_MapRGB(enemy05Surface->format, 255, 255, 255));
        m_enemy05Texture = SDL_CreateTextureFromSurface(m_renderer, enemy05Surface);
        SDL_FreeSurface(enemy05Surface);
        if (m_enemy05Texture) {
            SDL_Log("INFO: Loaded enemy_05.png: %s", enemy05Path.c_str());
            SDL_SetTextureBlendMode(m_enemy05Texture, SDL_BLENDMODE_BLEND);
        }
    } else {
        SDL_Log("INFO: Failed to load enemy_05.png: %s (path: %s)", IMG_GetError(), enemy05Path.c_str());
    }
    
    // Legacy: Load enemy_02 as default
    m_enemyTexture = m_enemy02Texture;
    
    // Load boss sprites
    std::string boss01Path = getResourcePath("boss01.png");
    SDL_Surface* boss01Surface = IMG_Load(boss01Path.c_str());
    if (boss01Surface) {
        SDL_SetColorKey(boss01Surface, SDL_TRUE, SDL_MapRGB(boss01Surface->format, 255, 255, 255));
        m_boss01Texture = SDL_CreateTextureFromSurface(m_renderer, boss01Surface);
        SDL_FreeSurface(boss01Surface);
        if (m_boss01Texture) {
            SDL_Log("INFO: Loaded boss01.png: %s", boss01Path.c_str());
            SDL_SetTextureBlendMode(m_boss01Texture, SDL_BLENDMODE_BLEND);
        }
    } else {
        SDL_Log("INFO: Failed to load boss01.png: %s (path: %s)", IMG_GetError(), boss01Path.c_str());
    }
    
    // Load boom explosion sprites
    std::string boom01Path = getResourcePath("boom01.png");
    SDL_Surface* boom01Surface = IMG_Load(boom01Path.c_str());
    if (boom01Surface) {
        SDL_SetColorKey(boom01Surface, SDL_TRUE, SDL_MapRGB(boom01Surface->format, 255, 255, 255));
        m_boom01Texture = SDL_CreateTextureFromSurface(m_renderer, boom01Surface);
        SDL_FreeSurface(boom01Surface);
        if (m_boom01Texture) {
            SDL_Log("INFO: Loaded boom01.png: %s", boom01Path.c_str());
            SDL_SetTextureBlendMode(m_boom01Texture, SDL_BLENDMODE_BLEND);
        }
    } else {
        SDL_Log("INFO: Failed to load boom01.png: %s (path: %s)", IMG_GetError(), boom01Path.c_str());
    }
    
    std::string boom02Path = getResourcePath("boom02.png");
    SDL_Surface* boom02Surface = IMG_Load(boom02Path.c_str());
    if (boom02Surface) {
        SDL_SetColorKey(boom02Surface, SDL_TRUE, SDL_MapRGB(boom02Surface->format, 255, 255, 255));
        m_boom02Texture = SDL_CreateTextureFromSurface(m_renderer, boom02Surface);
        SDL_FreeSurface(boom02Surface);
        if (m_boom02Texture) {
            SDL_Log("INFO: Loaded boom02.png: %s", boom02Path.c_str());
            SDL_SetTextureBlendMode(m_boom02Texture, SDL_BLENDMODE_BLEND);
        }
    } else {
        SDL_Log("INFO: Failed to load boom02.png: %s (path: %s)", IMG_GetError(), boom02Path.c_str());
    }
    
    std::string boom03Path = getResourcePath("boom03.png");
    SDL_Surface* boom03Surface = IMG_Load(boom03Path.c_str());
    if (boom03Surface) {
        SDL_SetColorKey(boom03Surface, SDL_TRUE, SDL_MapRGB(boom03Surface->format, 255, 255, 255));
        m_boom03Texture = SDL_CreateTextureFromSurface(m_renderer, boom03Surface);
        SDL_FreeSurface(boom03Surface);
        if (m_boom03Texture) {
            SDL_Log("INFO: Loaded boom03.png: %s", boom03Path.c_str());
            SDL_SetTextureBlendMode(m_boom03Texture, SDL_BLENDMODE_BLEND);
        }
    } else {
        SDL_Log("INFO: Failed to load boom03.png: %s (path: %s)", IMG_GetError(), boom03Path.c_str());
    }
    
    std::string boom04Path = getResourcePath("boom04.png");
    SDL_Surface* boom04Surface = IMG_Load(boom04Path.c_str());
    if (boom04Surface) {
        SDL_SetColorKey(boom04Surface, SDL_TRUE, SDL_MapRGB(boom04Surface->format, 255, 255, 255));
        m_boom04Texture = SDL_CreateTextureFromSurface(m_renderer, boom04Surface);
        SDL_FreeSurface(boom04Surface);
        if (m_boom04Texture) {
            SDL_Log("INFO: Loaded boom04.png: %s", boom04Path.c_str());
            SDL_SetTextureBlendMode(m_boom04Texture, SDL_BLENDMODE_BLEND);
        }
    } else {
        SDL_Log("INFO: Failed to load boom04.png: %s (path: %s)", IMG_GetError(), boom04Path.c_str());
    }
    
    std::string boom05Path = getResourcePath("boom05.png");
    SDL_Surface* boom05Surface = IMG_Load(boom05Path.c_str());
    if (boom05Surface) {
        SDL_SetColorKey(boom05Surface, SDL_TRUE, SDL_MapRGB(boom05Surface->format, 255, 255, 255));
        m_boom05Texture = SDL_CreateTextureFromSurface(m_renderer, boom05Surface);
        SDL_FreeSurface(boom05Surface);
        if (m_boom05Texture) {
            SDL_Log("INFO: Loaded boom05.png: %s", boom05Path.c_str());
            SDL_SetTextureBlendMode(m_boom05Texture, SDL_BLENDMODE_BLEND);
        }
    } else {
        SDL_Log("INFO: Failed to load boom05.png: %s (path: %s)", IMG_GetError(), boom05Path.c_str());
    }
    
    std::string boom06Path = getResourcePath("boom06.png");
    SDL_Surface* boom06Surface = IMG_Load(boom06Path.c_str());
    if (boom06Surface) {
        SDL_SetColorKey(boom06Surface, SDL_TRUE, SDL_MapRGB(boom06Surface->format, 255, 255, 255));
        m_boom06Texture = SDL_CreateTextureFromSurface(m_renderer, boom06Surface);
        SDL_FreeSurface(boom06Surface);
        if (m_boom06Texture) {
            SDL_Log("INFO: Loaded boom06.png: %s", boom06Path.c_str());
            SDL_SetTextureBlendMode(m_boom06Texture, SDL_BLENDMODE_BLEND);
        }
    } else {
        SDL_Log("INFO: Failed to load boom06.png: %s (path: %s)", IMG_GetError(), boom06Path.c_str());
    }
    
    // Load item box textures
    std::string itemBoxPath = getResourcePath("item_b.png");
    SDL_Surface* itemBoxSurface = IMG_Load(itemBoxPath.c_str());
    if (itemBoxSurface) {
        m_itemBoxTexture = SDL_CreateTextureFromSurface(m_renderer, itemBoxSurface);
        SDL_FreeSurface(itemBoxSurface);
        if (m_itemBoxTexture) {
            SDL_Log("INFO: Loaded item_b.png: %s", itemBoxPath.c_str());
        }
    }
    
    std::string itemSPath = getResourcePath("item_s.png");
    SDL_Surface* itemSSurface = IMG_Load(itemSPath.c_str());
    if (itemSSurface) {
        m_itemSTexture = SDL_CreateTextureFromSurface(m_renderer, itemSSurface);
        SDL_FreeSurface(itemSSurface);
        if (m_itemSTexture) {
            SDL_Log("INFO: Loaded item_s.png: %s", itemSPath.c_str());
        }
    }
    
    std::string itemLPath = getResourcePath("item_l.png");
    SDL_Surface* itemLSurface = IMG_Load(itemLPath.c_str());
    if (itemLSurface) {
        m_itemLTexture = SDL_CreateTextureFromSurface(m_renderer, itemLSurface);
        SDL_FreeSurface(itemLSurface);
        if (m_itemLTexture) {
            SDL_Log("INFO: Loaded item_l.png: %s", itemLPath.c_str());
        }
    }
    
    std::string itemMPath = getResourcePath("item_m.png");
    SDL_Surface* itemMSurface = IMG_Load(itemMPath.c_str());
    if (itemMSurface) {
        m_itemMTexture = SDL_CreateTextureFromSurface(m_renderer, itemMSurface);
        SDL_FreeSurface(itemMSurface);
        if (m_itemMTexture) {
            SDL_Log("INFO: Loaded item_m.png: %s", itemMPath.c_str());
        }
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
    
    // Load UI font (for score, countdown) - smaller size
    m_uiFont = TTF_OpenFont("C:\\Windows\\Fonts\\arial.ttf", 24);
    if (!m_uiFont) {
        SDL_Log("INFO: Failed to load UI font: %s", TTF_GetError());
    }
    
    // Load subtitle font - smaller size
    m_subtitleFont = TTF_OpenFont("C:\\Windows\\Fonts\\arial.ttf", 18);
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
            // ESC to toggle pause during gameplay
            if (event.key.keysym.sym == SDLK_ESCAPE) {
                if (m_gameState == GameState::PLAYING) {
                    m_gameState = GameState::PAUSED;
                    SDL_Log("INFO: Game paused");
                } else if (m_gameState == GameState::PAUSED) {
                    m_gameState = GameState::PLAYING;
                    SDL_Log("INFO: Game resumed");
                }
            }
            // Ctrl+C to toggle mouse grab
            else if (event.key.keysym.sym == SDLK_c && (SDL_GetModState() & KMOD_CTRL)) {
                m_mouseGrabbed = !m_mouseGrabbed;
                SDL_SetWindowGrab(m_window, m_mouseGrabbed ? SDL_TRUE : SDL_FALSE);
                SDL_Log("INFO: Mouse grab %s", m_mouseGrabbed ? "enabled" : "disabled");
            }
            // Any key to start from START_SCREEN - go directly to countdown
            else if (m_gameState == GameState::START_SCREEN) {
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
            // Any key to start from GAME_OVER
            else if (m_gameState == GameState::GAME_OVER) {
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
                m_mousePressed = true;
                
                // Click on START_SCREEN to start countdown directly
                if (m_gameState == GameState::START_SCREEN) {
                    m_gameState = GameState::COUNTDOWN;
                    m_stateTimer = 2.0f;  // 2 seconds: 1s GET READY, 1s GO
                    m_lives = 3;
                    m_score = 0;
                    m_enemies.clear();
                    m_bullets.clear();
                    int windowWidth, windowHeight;
                    SDL_GetWindowSize(m_window, &windowWidth, &windowHeight);
                    m_player = std::make_unique<Player>(windowWidth / 2.0f, windowHeight - 80.0f);
                }
                // Click on GAME_OVER to start countdown
                else if (m_gameState == GameState::GAME_OVER) {
                    m_gameState = GameState::COUNTDOWN;
                    m_stateTimer = 2.0f;  // 2 seconds: 1s GET READY, 1s GO
                    m_lives = 3;
                    m_score = 0;
                    m_enemies.clear();
                    m_bullets.clear();
                    int windowWidth, windowHeight;
                    SDL_GetWindowSize(m_window, &windowWidth, &windowHeight);
                    m_player = std::make_unique<Player>(windowWidth / 2.0f, windowHeight - 80.0f);
                }
                // Fire missiles with mouse click in PLAYING state
                else if (m_gameState == GameState::PLAYING) {
                    // Missile firing is now handled in update() with auto-fire
                }
            }
        }
        else if (event.type == SDL_MOUSEBUTTONUP) {
            if (event.button.button == SDL_BUTTON_LEFT) {
                m_mousePressed = false;
            }
        }
    }
}

void Game::update(float deltaTime) {
    // Don't update when paused
    if (m_gameState == GameState::PAUSED) {
        return;
    }
    
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
            
            // Switch from opening music to stage music
            if (m_stage01Music) {
                Mix_HaltMusic();
                Mix_VolumeMusic(64);
                Mix_PlayMusic(m_stage01Music, -1);
                SDL_Log("INFO: Game started - Playing stage01.wav");
            }
            
            // Item boxes will spawn automatically in space city section
        }
        return;
    }
    else if (m_gameState == GameState::GAME_OVER) {
        // Do nothing in game over state
        return;
    }
    
    // Execute game logic only in PLAYING state
    // Background scroll with seamless transitions
    m_backgroundY1 += m_backgroundScrollSpeed * deltaTime;
    m_backgroundY2 += m_backgroundScrollSpeed * deltaTime;
    int windowHeight;
    SDL_GetWindowSize(m_window, nullptr, &windowHeight);
    
    // Advance to next background in sequence
    auto advanceSequence = [&]() -> SDL_Texture* {
        SDL_Texture* textureToUse = m_nextSequenceTexture;
        
        // Determine what comes next after this one
        if (m_boss) {
            // Boss active: keep looping background 01
            m_nextSequenceTexture = m_background01Texture;
        } else if (m_enemyKillCount >= 18) {
            // Before boss: transition to 04 then 01
            if (m_bgSequenceIndex == 2) {
                // From bg03 to bg04
                m_bgSequenceIndex = 4;
                m_nextSequenceTexture = m_background04Texture;
                SDL_Log("INFO: Next background: 04 (Before Boss)");
            } else if (m_bgSequenceIndex == 4) {
                // From bg04 to bg01 (boss)
                m_bgSequenceIndex = 5;
                m_nextSequenceTexture = m_background01Texture;
                m_bg04Completed = true;  // Mark bg04 as completed
                SDL_Log("INFO: Next background: 01 (Boss) - Starting 10s timer");
            } else if (m_bgSequenceIndex == 5) {
                // Boss: loop bg01
                m_nextSequenceTexture = m_background01Texture;
            }
        } else {
            // Normal sequence: 01 -> 02 -> 03 (loop)
            if (m_bgSequenceIndex == 0) {
                // After 01 comes 02, prepare 03 next
                m_bgSequenceIndex = 1;
                m_nextSequenceTexture = m_background03Texture;
                SDL_Log("INFO: Next background: 03");
            } else if (m_bgSequenceIndex == 1) {
                // After 02 comes 03, keep looping 03
                m_bgSequenceIndex = 2;
                m_bg03LoopCount = 0;
                m_nextSequenceTexture = m_background03Texture;
                SDL_Log("INFO: Next background: 03 (first loop)");
            } else if (m_bgSequenceIndex == 2) {
                // bg03 loops
                m_bg03LoopCount++;
                m_nextSequenceTexture = m_background03Texture;
                SDL_Log("INFO: Next background: 03 (loop %d)", m_bg03LoopCount);
            }
        }
        
        return textureToUse;
    };
    
    // When top background scrolls off screen, replace it with next in sequence
    if (m_backgroundY1 >= windowHeight) {
        m_backgroundY1 = m_backgroundY2 - windowHeight;
        m_bgSlot1Texture = advanceSequence();
    }
    
    // When bottom background scrolls off screen, replace it with next in sequence
    if (m_backgroundY2 >= windowHeight) {
        m_backgroundY2 = m_backgroundY1 - windowHeight;
        m_bgSlot2Texture = advanceSequence();
    }
    
    // Track space city section and spawn item boxes
    // Space city is bg02, bg03 (all), and bg04
    bool wasInSpaceCity = m_inSpaceCity;
    
    // Simply check if we're in the space city backgrounds (index 1, 2, or 4)
    if (m_bgSequenceIndex == 1 || m_bgSequenceIndex == 2 || m_bgSequenceIndex == 4) {
        m_inSpaceCity = true;
    } else {
        m_inSpaceCity = false;
    }
    
    // Log when entering space city
    if (m_inSpaceCity && !wasInSpaceCity) {
        SDL_Log("INFO: Entering space city section (index %d)", m_bgSequenceIndex);
    }
    
    // Spawn item boxes periodically in space city
    if (m_inSpaceCity) {
        m_itemBoxSpawnTimer += deltaTime;
        if (m_itemBoxSpawnTimer >= 2.0f) {  // Spawn every 2 seconds
            int windowWidth;
            SDL_GetWindowSize(m_window, &windowWidth, nullptr);
            
            // Spawn 2-3 boxes
            int boxCount = 2 + (rand() % 2);
            for (int i = 0; i < boxCount; i++) {
                float boxX = 50.0f + (rand() % (windowWidth - 100));
                float boxY = -50.0f - (i * 80.0f);  // Stagger slightly
                m_itemBoxes.push_back(std::make_unique<ItemBox>(boxX, boxY));
            }
            
            SDL_Log("INFO: Spawned %d item boxes (total: %zu)", boxCount, m_itemBoxes.size());
            m_itemBoxSpawnTimer = 0.0f;
        }
    } else if (wasInSpaceCity && !m_inSpaceCity) {
        // Just exited space city, reset timer
        SDL_Log("INFO: Exiting space city section");
        m_itemBoxSpawnTimer = 0.0f;
    }
    
    // Update player
    m_player->update(deltaTime);
    
    // Check player screen boundaries
    int windowWidth;
    SDL_GetWindowSize(m_window, &windowWidth, &windowHeight);
    m_player->clampToScreen(windowWidth, windowHeight);
    
    // Auto-fire bullets
    if (m_player->canShoot()) {
        // Laser count based on laser level (0=1, 1=2, 2=3, 3=3)
        int laserCount = 1 + m_player->getLaserLevel();
        if (laserCount > 3) laserCount = 3;
        
        float playerCenterX = m_player->getX() + m_player->getWidth() / 2;
        
        // Fire lasers based on level
        if (laserCount == 1) {
            m_bullets.push_back(std::make_unique<Bullet>(
                playerCenterX, m_player->getY(), Bullet::Owner::PLAYER
            ));
        } else {
            float spacing = 20.0f;
            float startX = playerCenterX - (laserCount - 1) * spacing / 2;
            for (int i = 0; i < laserCount; i++) {
                m_bullets.push_back(std::make_unique<Bullet>(
                    startX + i * spacing, m_player->getY(), Bullet::Owner::PLAYER
                ));
            }
        }
        
        m_player->resetShootTimer();
        
        // Play shoot sound
        if (m_shootSound) {
            Mix_VolumeChunk(m_shootSound, 32);  // Reduce volume (0-128, default 128)
            Mix_PlayChannel(-1, m_shootSound, 0);
        } else {
            // Fallback: Windows beep sound
            #ifdef _WIN32
            Beep(800, 50);
            #endif
        }
    }
    
    // Auto-fire missiles when mouse is pressed (every 1.0 second, 2x slower than laser)
    m_missileShootTimer -= deltaTime;
    if (m_mousePressed && m_missileShootTimer <= 0.0f) {
        // Missile count based on missile level (0=1, 1=2, 2=3, 3=4, 4=5)
        int missileCount = 1 + m_player->getMissileLevel();
        if (missileCount > 5) missileCount = 5;
        
        float playerCenterX = m_player->getX() + m_player->getWidth() / 2;
        
        // Fire missiles based on level
        if (missileCount == 1) {
            m_bullets.push_back(std::make_unique<Bullet>(
                playerCenterX - 7.5f, m_player->getY(), Bullet::Owner::PLAYER, Bullet::BulletType::MISSILE
            ));
        } else {
            float spacing = 30.0f;
            float startX = playerCenterX - (missileCount - 1) * spacing / 2;
            for (int i = 0; i < missileCount; i++) {
                m_bullets.push_back(std::make_unique<Bullet>(
                    startX + i * spacing - 7.5f, m_player->getY(), Bullet::Owner::PLAYER, Bullet::BulletType::MISSILE
                ));
            }
        }
        
        m_missileShootTimer = m_missileShootCooldown;  // Reset to 1.0 second
        
        // Play shoot sound (same as laser)
        if (m_shootSound) {
            Mix_VolumeChunk(m_shootSound, 32);
            Mix_PlayChannel(-1, m_shootSound, 0);
        }
    }

    // Spawn enemies (only if no boss active)
    if (!m_boss) {
        m_enemySpawnTimer += deltaTime;
        if (m_enemySpawnTimer >= m_enemySpawnInterval) {
            int windowWidth, windowHeight;
            SDL_GetWindowSize(m_window, &windowWidth, &windowHeight);
            
            float enemyX = static_cast<float>(rand() % (windowWidth - 40));
            
            // Determine enemy type (10% special, 90% random types)
            int typeRoll = rand() % 100;
            Enemy::EnemyType enemyType;
            bool isSpecial = false;
            
            if (typeRoll < 10) {
                // 10% chance: Special enemy (TYPE_03)
                enemyType = Enemy::EnemyType::TYPE_03;
                isSpecial = true;
            } else {
                // 90% chance: Random type from TYPE_01, 02, 04, 05
                int typeIndex = rand() % 4;
                switch (typeIndex) {
                    case 0: enemyType = Enemy::EnemyType::TYPE_01; break;
                    case 1: enemyType = Enemy::EnemyType::TYPE_02; break;
                    case 2: enemyType = Enemy::EnemyType::TYPE_04; break;
                    case 3: enemyType = Enemy::EnemyType::TYPE_05; break;
                }
            }
            
            m_enemies.push_back(std::make_unique<Enemy>(enemyX, -40.0f, enemyType, isSpecial));
            m_enemySpawnTimer = 0.0f;
        }
    }
    
    // Update boss spawn timer after bg04
    if (m_bg04Completed && !m_boss) {
        m_bossSpawnTimer += deltaTime;
        if (m_bossSpawnTimer >= 10.0f) {
            spawnBoss(m_currentStage);
            m_enemyKillCount = 0;
            m_enemies.clear();  // Clear all enemies when boss appears
            m_bg04Completed = false;  // Reset for next stage
            m_bossSpawnTimer = 0.0f;
        }
    }

    // Update boss
    if (m_boss) {
        m_boss->update(deltaTime);
        
        // Boss shoots occasionally
        if (m_boss->canShoot() && m_boss->getState() == Boss::BossState::FIGHTING) {
            // Boss shoots from 12 cannon positions
            const Boss::CannonPos* cannons = m_boss->getCannonPositions();
            int cannonCount = m_boss->getCannonCount();
            
            for (int i = 0; i < cannonCount; i++) {
                float cannonX = m_boss->getX() + cannons[i].offsetX;
                float cannonY = m_boss->getY() + cannons[i].offsetY;
                
                m_enemyBullets.push_back(std::make_unique<Bullet>(
                    cannonX, cannonY, Bullet::Owner::ENEMY
                ));
            }
            
            m_boss->resetShootTimer();
        }
        
        // Remove boss if dead
        if (m_boss->isOffScreen()) {
            m_boss.reset();
            m_currentStage++;
            
            // Resume stage music based on current stage
            Mix_HaltMusic();
            Mix_Music* stageMusic = nullptr;
            if (m_currentStage == 1 && m_stage01Music) {
                stageMusic = m_stage01Music;
            } else if (m_currentStage == 2 && m_stage02Music) {
                stageMusic = m_stage02Music;
            } else if (m_bgMusic) {
                stageMusic = m_bgMusic;  // Fallback to default music
            }
            
            if (stageMusic) {
                Mix_VolumeMusic(64);
                Mix_PlayMusic(stageMusic, -1);
            }
        }
    }

    // Update enemies
    float playerCenterX = m_player->getX() + m_player->getWidth() / 2;
    for (auto& enemy : m_enemies) {
        enemy->update(deltaTime, playerCenterX);
        
        // Enemy shoots occasionally
        if (enemy->canShoot()) {
            float enemyCenterX = enemy->getX() + enemy->getWidth() / 2;
            float enemyBottomY = enemy->getY() + enemy->getHeight();
            
            // TYPE_05: Burst shooting (3 bullets)
            if (enemy->getType() == Enemy::EnemyType::TYPE_05 && enemy->getBurstCount() > 0) {
                m_enemyBullets.push_back(std::make_unique<Bullet>(
                    enemyCenterX, enemyBottomY, Bullet::Owner::ENEMY
                ));
                enemy->decreaseBurstCount();
                
                // Continue burst (0.2s delay between bullets)
                if (enemy->getBurstCount() > 0) {
                    enemy->resetShootTimer();
                    // Short delay for burst
                    // Note: This is handled in Enemy class with resetShootTimer
                }
            } else {
                // Normal shooting for other types
                m_enemyBullets.push_back(std::make_unique<Bullet>(
                    enemyCenterX, enemyBottomY, Bullet::Owner::ENEMY
                ));
                enemy->resetShootTimer();
            }
        }
    }
    
    // Update enemy bullets
    for (auto& bullet : m_enemyBullets) {
        bullet->update(deltaTime);
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
            [this](const std::unique_ptr<Bullet>& bullet) {
                return bullet->isOffScreen(getWindowHeight());
            }),
        m_bullets.end()
    );
    
    // Remove enemy bullets that went off screen
    m_enemyBullets.erase(
        std::remove_if(m_enemyBullets.begin(), m_enemyBullets.end(),
            [this](const std::unique_ptr<Bullet>& bullet) {
                return bullet->isOffScreen(getWindowHeight());
            }),
        m_enemyBullets.end()
    );
    
    // Remove enemy bullets that went off screen
    m_enemyBullets.erase(
        std::remove_if(m_enemyBullets.begin(), m_enemyBullets.end(),
            [this](const std::unique_ptr<Bullet>& bullet) {
                return bullet->isOffScreen(getWindowHeight());
            }),
        m_enemyBullets.end()
    );

    // Collision detection (simple AABB)
    for (auto bulletIt = m_bullets.begin(); bulletIt != m_bullets.end();) {
        bool bulletRemoved = false;
        
        if ((*bulletIt)->getOwner() == Bullet::Owner::PLAYER) {
            // Missiles cannot destroy flying enemies (only ground objects in future)
            if ((*bulletIt)->getType() == Bullet::BulletType::MISSILE) {
                ++bulletIt;
                continue;
            }
            
            // Only lasers can destroy enemies
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
                    
                    // Increment enemy kill count
                    m_enemyKillCount++;
                    
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
    
    // Update power-ups
    for (auto& powerUp : m_powerUps) {
        powerUp->update(deltaTime);
    }
    
    // Remove power-ups that went off screen
    m_powerUps.erase(
        std::remove_if(m_powerUps.begin(), m_powerUps.end(),
            [](const std::unique_ptr<PowerUp>& powerUp) {
                return powerUp->isOffScreen();
            }),
        m_powerUps.end()
    );
    
    // Update item boxes
    for (auto& itemBox : m_itemBoxes) {
        itemBox->update(deltaTime);
    }
    
    // Remove item boxes that went off screen
    m_itemBoxes.erase(
        std::remove_if(m_itemBoxes.begin(), m_itemBoxes.end(),
            [](const std::unique_ptr<ItemBox>& box) {
                return box->isOffScreen();
            }),
        m_itemBoxes.end()
    );
    
    // Check player-enemy collision
    checkPlayerEnemyCollision();
    
    // Check player-enemy bullet collision
    checkPlayerEnemyBulletCollision();
    
    // Check player-boss collision
    checkPlayerBossCollision();
    
    // Check boss-bullet collision
    checkBossBulletCollision();
    
    // Check power-up collection
    checkPowerUpCollection();
    
    // Check item box collection
    checkItemBoxCollection();
    
    // Check missile-item box collision
    checkMissileItemBoxCollision();
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
            
            // Collision occurred: remove enemy and damage player (instant explosion)
            enemyIt = m_enemies.erase(enemyIt);
            damagePlayer();
            return;
        } else {
            ++enemyIt;
        }
    }
}

void Game::checkPlayerEnemyBulletCollision() {
    for (auto bulletIt = m_enemyBullets.begin(); bulletIt != m_enemyBullets.end();) {
        // Use hitbox instead of full sprite for more accurate collision
        float playerLeft = m_player->getHitboxX();
        float playerRight = playerLeft + m_player->getHitboxWidth();
        float playerTop = m_player->getHitboxY();
        float playerBottom = playerTop + m_player->getHitboxHeight();
        
        float bulletLeft = (*bulletIt)->getX();
        float bulletRight = bulletLeft + (*bulletIt)->getWidth();
        float bulletTop = (*bulletIt)->getY();
        float bulletBottom = bulletTop + (*bulletIt)->getHeight();
        
        // Check collision between hitboxes
        if (playerLeft < bulletRight &&
            playerRight > bulletLeft &&
            playerTop < bulletBottom &&
            playerBottom > bulletTop) {
            
            // Remove bullet
            bulletIt = m_enemyBullets.erase(bulletIt);
            
            // Damage player (-1 energy)
            if (m_player->takeDamage(1)) {
                // Energy reached 0: explode
                damagePlayer();
                return;
            }
        } else {
            ++bulletIt;
        }
    }
}

void Game::damagePlayer() {
    m_lives--;
    
    // Show explosion animation (boom01-06) with sound
    SDL_Texture* boomFrames[] = {m_boom01Texture, m_boom02Texture, m_boom03Texture, m_boom04Texture, m_boom05Texture, m_boom06Texture};
            float playerCenterX = m_player->getX() + m_player->getWidth() / 2;
            float playerCenterY = m_player->getY() + m_player->getHeight() / 2;
            
            // Play explosion sound at the start of animation
            if (m_explosionSound) {
                Mix_PlayChannel(-1, m_explosionSound, 0);
            } else {
                // Fallback: Windows beep sound
                #ifdef _WIN32
                Beep(300, 200);
                #endif
            }
            
            for (int i = 0; i < 6; i++) {
                if (boomFrames[i]) {
                    // Clear screen
                    SDL_SetRenderDrawColor(m_renderer, 0, 0, 0, 255);
                    SDL_RenderClear(m_renderer);
                    
                    // Draw background using slot system
                    SDL_Texture* bg1Texture = m_bgSlot1Texture ? m_bgSlot1Texture : m_backgroundTexture;
                    SDL_Texture* bg2Texture = m_bgSlot2Texture ? m_bgSlot2Texture : m_backgroundTexture;
                    
                    if (bg1Texture && bg2Texture) {
                        int windowWidth, windowHeight;
                        SDL_GetWindowSize(m_window, &windowWidth, &windowHeight);
                        SDL_Rect bg1 = {0, static_cast<int>(m_backgroundY1), windowWidth, windowHeight};
                        SDL_Rect bg2 = {0, static_cast<int>(m_backgroundY2), windowWidth, windowHeight};
                        SDL_RenderCopy(m_renderer, bg1Texture, nullptr, &bg1);
                        SDL_RenderCopy(m_renderer, bg2Texture, nullptr, &bg2);
                    }
                    
                    // Draw explosion frame centered on player
                    int boomSize = 80;  // Match player size
                    SDL_Rect boomRect = {
                        static_cast<int>(playerCenterX - boomSize / 2),
                        static_cast<int>(playerCenterY - boomSize / 2),
                        boomSize,
                        boomSize
                    };
                    SDL_RenderCopy(m_renderer, boomFrames[i], nullptr, &boomRect);
                    
                    SDL_RenderPresent(m_renderer);
                    SDL_Delay(100); // 0.1 second per frame
                }
            }
            
            // If lives reach 0, game over
            if (m_lives <= 0) {
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
                m_enemyBullets.clear();
            }
            else {
                // If lives remain, restart after countdown
                m_gameState = GameState::COUNTDOWN;
                m_stateTimer = 2.0f; // 2 seconds: 1s GET READY, 1s GO
                m_enemies.clear();
                m_bullets.clear();
                m_enemyBullets.clear();
                m_powerUps.clear();
                int windowWidth, windowHeight;
                SDL_GetWindowSize(m_window, &windowWidth, &windowHeight);
                m_player = std::make_unique<Player>(windowWidth / 2.0f, windowHeight - 80.0f);
                m_player->resetOnDeath(); // Reset power-ups based on Keep flags
            }
}

void Game::checkPlayerBossCollision() {
    if (!m_boss) return;
    
    // Use hitbox for collision
    float playerLeft = m_player->getHitboxX();
    float playerRight = playerLeft + m_player->getHitboxWidth();
    float playerTop = m_player->getHitboxY();
    float playerBottom = playerTop + m_player->getHitboxHeight();
    
    float bossLeft = m_boss->getX();
    float bossRight = bossLeft + m_boss->getWidth();
    float bossTop = m_boss->getY();
    float bossBottom = bossTop + m_boss->getHeight();
    
    // Check collision between player and boss
    if (playerLeft < bossRight &&
        playerRight > bossLeft &&
        playerTop < bossBottom &&
        playerBottom > bossTop) {
        
        // Player collided with boss: instant death
        damagePlayer();
    }
}

void Game::checkBossBulletCollision() {
    if (!m_boss || m_boss->getState() != Boss::BossState::FIGHTING) {
        return;
    }
    
    for (auto bulletIt = m_bullets.begin(); bulletIt != m_bullets.end();) {
        bool bulletRemoved = false;
        
        if ((*bulletIt)->getOwner() == Bullet::Owner::PLAYER) {
            int damage = 1;
            bool hitBoss = false;
            
            // Check weak point first (front center) - 5x damage
            float weakX = m_boss->getWeakPointX();
            float weakY = m_boss->getWeakPointY();
            float weakW = m_boss->getWeakPointWidth();
            float weakH = m_boss->getWeakPointHeight();
            
            if ((*bulletIt)->getX() < weakX + weakW &&
                (*bulletIt)->getX() + (*bulletIt)->getWidth() > weakX &&
                (*bulletIt)->getY() < weakY + weakH &&
                (*bulletIt)->getY() + (*bulletIt)->getHeight() > weakY) {
                
                // Hit weak point: 5x damage
                damage = 5;
                hitBoss = true;
            }
            // Check collision with boss body (normal hitbox)
            else if ((*bulletIt)->getX() < m_boss->getHitboxX() + m_boss->getHitboxWidth() &&
                     (*bulletIt)->getX() + (*bulletIt)->getWidth() > m_boss->getHitboxX() &&
                     (*bulletIt)->getY() < m_boss->getHitboxY() + m_boss->getHitboxHeight() &&
                     (*bulletIt)->getY() + (*bulletIt)->getHeight() > m_boss->getHitboxY()) {
                
                // Normal damage
                damage = 1;
                hitBoss = true;
            }
            
            if (hitBoss) {
                // Damage boss
                m_boss->takeDamage(damage);
                
                // Add score
                if (m_boss->getHealth() <= 0) {
                    m_score += 1000;  // Big bonus for defeating boss
                } else {
                    m_score += (damage == 5) ? 25 : 5;  // More score for weak point hit
                }
                
                if (m_score > m_highScore) {
                    m_highScore = m_score;
                }
                
                bulletIt = m_bullets.erase(bulletIt);
                bulletRemoved = true;
            }
        }
        
        if (!bulletRemoved) {
            ++bulletIt;
        }
    }
}

void Game::spawnBoss(int stage) {
    int windowWidth, windowHeight;
    SDL_GetWindowSize(m_window, &windowWidth, &windowHeight);
    
    float bossX = windowWidth / 2.0f - 120.0f;  // Center horizontally (boss is 240 wide)
    float bossY = -250.0f;  // Start above screen
    
    m_boss = std::make_unique<Boss>(bossX, bossY, stage);
    
    // Stop normal background music and play boss music
    Mix_HaltMusic();
    
    Mix_Music* bossMusic = nullptr;
    switch (stage) {
        case 1:
            bossMusic = m_boss01Music;
            break;
        case 2:
            bossMusic = m_boss02Music;
            break;
        case 3:
            bossMusic = m_boss03Music;
            break;
        case 4:
            bossMusic = m_boss04Music;
            break;
        default:
            // Cycle through boss music if stage > 4
            int musicIndex = ((stage - 1) % 4) + 1;
            switch (musicIndex) {
                case 1: bossMusic = m_boss01Music; break;
                case 2: bossMusic = m_boss02Music; break;
                case 3: bossMusic = m_boss03Music; break;
                case 4: bossMusic = m_boss04Music; break;
            }
            break;
    }
    
    if (bossMusic) {
        Mix_VolumeMusic(128);
        if (Mix_PlayMusic(bossMusic, -1) == -1) {
            SDL_Log("ERROR: Failed to play boss music: %s", Mix_GetError());
        } else {
            SDL_Log("INFO: Boss Stage %d spawned! Playing boss%02d music.", stage, stage);
        }
    } else {
        SDL_Log("WARNING: Boss Stage %d spawned! (No boss music available)", stage);
    }
}

void Game::render() {
    // Clear screen (black)
    SDL_SetRenderDrawColor(m_renderer, 0, 0, 0, 255);
    SDL_RenderClear(m_renderer);
    
    // Draw background using slot system (first)
    SDL_Texture* bg1Texture = m_bgSlot1Texture ? m_bgSlot1Texture : m_backgroundTexture;
    SDL_Texture* bg2Texture = m_bgSlot2Texture ? m_bgSlot2Texture : m_backgroundTexture;
    
    if (bg1Texture && bg2Texture) {
        int windowWidth, windowHeight;
        SDL_GetWindowSize(m_window, &windowWidth, &windowHeight);
        SDL_Rect bg1 = {0, static_cast<int>(m_backgroundY1), windowWidth, windowHeight};
        SDL_Rect bg2 = {0, static_cast<int>(m_backgroundY2), windowWidth, windowHeight};
        SDL_RenderCopy(m_renderer, bg1Texture, nullptr, &bg1);
        SDL_RenderCopy(m_renderer, bg2Texture, nullptr, &bg2);
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
        // Render player (using movement state texture)
        SDL_Texture* currentShipTexture = m_shipStopTexture;
        if (m_player->getMovementState() == Player::MovementState::FORWARD) {
            currentShipTexture = m_shipForwardTexture;
        } else if (m_player->getMovementState() == Player::MovementState::BACKWARD) {
            currentShipTexture = m_shipBackwardTexture;
        } else if (m_player->getMovementState() == Player::MovementState::LEFT) {
            currentShipTexture = m_shipLeftTexture;
        } else if (m_player->getMovementState() == Player::MovementState::RIGHT) {
            currentShipTexture = m_shipRightTexture;
        }
        
        if (currentShipTexture) {
            m_player->render(m_renderer, currentShipTexture, nullptr);
        } else if (m_playerTexture) {
            m_player->render(m_renderer, m_playerTexture, nullptr);
        } else {
            m_player->render(m_renderer);
        }

        // Render enemies (using different textures based on type)
        for (auto& enemy : m_enemies) {
            SDL_Texture* enemyTexture = nullptr;
            
            // Select texture based on enemy type
            switch (enemy->getType()) {
                case Enemy::EnemyType::TYPE_01:
                    enemyTexture = m_enemy01Texture;
                    break;
                case Enemy::EnemyType::TYPE_02:
                    enemyTexture = m_enemy02Texture;
                    break;
                case Enemy::EnemyType::TYPE_03:
                    enemyTexture = m_enemy03Texture;
                    break;
                case Enemy::EnemyType::TYPE_04:
                    enemyTexture = m_enemy04Texture;
                    break;
                case Enemy::EnemyType::TYPE_05:
                    enemyTexture = m_enemy05Texture;
                    break;
            }
            
            if (enemyTexture) {
                enemy->render(m_renderer, enemyTexture, nullptr);
            } else {
                enemy->render(m_renderer);
            }
        }
        
        // Render boss
        if (m_boss && m_boss->getState() != Boss::BossState::DEAD) {
            if (m_boss01Texture) {
                m_boss->render(m_renderer, m_boss01Texture, nullptr);
            } else {
                m_boss->render(m_renderer);
            }
            
            // Render boss health bar
            int barWidth = 300;
            int barHeight = 20;
            int barX = (getWindowWidth() - barWidth) / 2;
            int barY = 10;
            
            // Background (red)
            SDL_SetRenderDrawColor(m_renderer, 100, 0, 0, 255);
            SDL_Rect bgRect = {barX, barY, barWidth, barHeight};
            SDL_RenderFillRect(m_renderer, &bgRect);
            
            // Health (green)
            float healthPercent = static_cast<float>(m_boss->getHealth()) / m_boss->getMaxHealth();
            SDL_SetRenderDrawColor(m_renderer, 0, 255, 0, 255);
            SDL_Rect healthRect = {barX, barY, static_cast<int>(barWidth * healthPercent), barHeight};
            SDL_RenderFillRect(m_renderer, &healthRect);
            
            // Border (white)
            SDL_SetRenderDrawColor(m_renderer, 255, 255, 255, 255);
            SDL_RenderDrawRect(m_renderer, &bgRect);
        }

        // Render bullets
        for (auto& bullet : m_bullets) {
            bullet->render(m_renderer);
        }
        
        // Render enemy bullets
        for (auto& bullet : m_enemyBullets) {
            bullet->render(m_renderer);
        }
        
        // Render power-ups
        for (auto& powerUp : m_powerUps) {
            powerUp->render(m_renderer, m_itemSTexture, m_itemLTexture, m_itemMTexture);
        }
        
        // Render item boxes
        for (auto& itemBox : m_itemBoxes) {
            itemBox->render(m_renderer, m_itemBoxTexture, m_itemSTexture, m_itemLTexture, m_itemMTexture);
        }
        
        // Render UI
        renderUI();
        
        // Show pause overlay if paused
        if (m_gameState == GameState::PAUSED) {
            // Semi-transparent overlay
            SDL_SetRenderDrawBlendMode(m_renderer, SDL_BLENDMODE_BLEND);
            SDL_SetRenderDrawColor(m_renderer, 0, 0, 0, 128);
            int windowWidth, windowHeight;
            SDL_GetWindowSize(m_window, &windowWidth, &windowHeight);
            SDL_Rect overlay = {0, 0, windowWidth, windowHeight};
            SDL_RenderFillRect(m_renderer, &overlay);
            
            // Render "PAUSED" text
            SDL_Color white = {255, 255, 255, 255};
            SDL_Surface* textSurface = TTF_RenderText_Solid(m_titleFont, "PAUSED", white);
            if (textSurface) {
                SDL_Texture* textTexture = SDL_CreateTextureFromSurface(m_renderer, textSurface);
                if (textTexture) {
                    SDL_Rect textRect = {
                        windowWidth / 2 - textSurface->w / 2,
                        windowHeight / 2 - textSurface->h / 2,
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
    else if (m_gameState == GameState::GAME_OVER) {
        renderGameOver();
    }

    // Present to screen
    SDL_RenderPresent(m_renderer);
}

void Game::renderUI() {
    int windowWidth, windowHeight;
    SDL_GetWindowSize(m_window, &windowWidth, &windowHeight);
    
    // Top left: Display score with 7-segment style
    SDL_Color scoreColor = {255, 255, 255, 255};  // White
    draw7SegmentNumber(m_score, 10, 10, 20, 30, 5, scoreColor);
    
    // Top center: Display high score with 7-segment style (gold)
    SDL_Color highScoreColor = {255, 215, 0, 255}; // Gold
    int highScoreWidth = static_cast<int>(std::to_string(m_highScore).length()) * 25;
    draw7SegmentNumber(m_highScore, (windowWidth - highScoreWidth) / 2, 10, 20, 30, 5, highScoreColor);
    
    // Top right: Display remaining lives with ship_stop icons
    if (m_shipStopTexture) {
        int shipIconSize = 24;  // Small ship icon size
        int spacing = 5;  // Spacing between icons
        int startX = windowWidth - (m_lives * (shipIconSize + spacing));
        
        for (int i = 0; i < m_lives; i++) {
            SDL_Rect shipRect = {
                startX + i * (shipIconSize + spacing),
                10,
                shipIconSize,
                shipIconSize
            };
            SDL_RenderCopy(m_renderer, m_shipStopTexture, nullptr, &shipRect);
        }
    }
    
    // Bottom left: Power-up collection counters with icons
    if (m_subtitleFont) {
        int iconSize = 32;  // Size of item icons
        int textOffsetX = iconSize + 10;  // Text starts after icon
        int lineHeight = 35;  // Space between lines
        int startY = windowHeight - 110;
        
        // Speed counter (Cyan) with icon
        if (m_itemSTexture) {
            SDL_Rect iconRect = {10, startY, iconSize, iconSize};
            SDL_RenderCopy(m_renderer, m_itemSTexture, nullptr, &iconRect);
        }
        std::string speedText = std::to_string(m_player->getSpeedCount()) + "/3 Lv." + std::to_string(m_player->getSpeedLevel());
        SDL_Color speedColor = {0, 255, 255, 255};
        SDL_Surface* speedSurface = TTF_RenderText_Blended(m_subtitleFont, speedText.c_str(), speedColor);
        if (speedSurface) {
            SDL_Texture* speedTexture = SDL_CreateTextureFromSurface(m_renderer, speedSurface);
            if (speedTexture) {
                SDL_Rect speedRect = {10 + textOffsetX, startY + (iconSize - speedSurface->h) / 2, speedSurface->w, speedSurface->h};
                SDL_RenderCopy(m_renderer, speedTexture, nullptr, &speedRect);
                SDL_DestroyTexture(speedTexture);
            }
            SDL_FreeSurface(speedSurface);
        }
        
        // Laser counter (Yellow) with icon
        if (m_itemLTexture) {
            SDL_Rect iconRect = {10, startY + lineHeight, iconSize, iconSize};
            SDL_RenderCopy(m_renderer, m_itemLTexture, nullptr, &iconRect);
        }
        std::string laserText = std::to_string(m_player->getLaserCount()) + "/3 Lv." + std::to_string(m_player->getLaserLevel());
        SDL_Color laserColor = {255, 255, 0, 255};
        SDL_Surface* laserSurface = TTF_RenderText_Blended(m_subtitleFont, laserText.c_str(), laserColor);
        if (laserSurface) {
            SDL_Texture* laserTexture = SDL_CreateTextureFromSurface(m_renderer, laserSurface);
            if (laserTexture) {
                SDL_Rect laserRect = {10 + textOffsetX, startY + lineHeight + (iconSize - laserSurface->h) / 2, laserSurface->w, laserSurface->h};
                SDL_RenderCopy(m_renderer, laserTexture, nullptr, &laserRect);
                SDL_DestroyTexture(laserTexture);
            }
            SDL_FreeSurface(laserSurface);
        }
        
        // Missile counter (Orange) with icon
        if (m_itemMTexture) {
            SDL_Rect iconRect = {10, startY + lineHeight * 2, iconSize, iconSize};
            SDL_RenderCopy(m_renderer, m_itemMTexture, nullptr, &iconRect);
        }
        std::string missileText = std::to_string(m_player->getMissileCount()) + "/3 Lv." + std::to_string(m_player->getMissileLevel());
        SDL_Color missileColor = {255, 165, 0, 255};
        SDL_Surface* missileSurface = TTF_RenderText_Blended(m_subtitleFont, missileText.c_str(), missileColor);
        if (missileSurface) {
            SDL_Texture* missileTexture = SDL_CreateTextureFromSurface(m_renderer, missileSurface);
            if (missileTexture) {
                SDL_Rect missileRect = {10 + textOffsetX, startY + lineHeight * 2 + (iconSize - missileSurface->h) / 2, missileSurface->w, missileSurface->h};
                SDL_RenderCopy(m_renderer, missileTexture, nullptr, &missileRect);
                SDL_DestroyTexture(missileTexture);
            }
            SDL_FreeSurface(missileSurface);
        }
    }
    
    // Bottom right: Energy bar
    // Energy bar background
    SDL_SetRenderDrawColor(m_renderer, 50, 50, 50, 255);
    SDL_Rect energyBg = {windowWidth - 210, windowHeight - 40, 200, 30};
    SDL_RenderFillRect(m_renderer, &energyBg);
    
    // Energy bar border
    SDL_SetRenderDrawColor(m_renderer, 255, 255, 255, 255);
    SDL_RenderDrawRect(m_renderer, &energyBg);
    
    // Energy bar fill
    int energyWidth = static_cast<int>(196.0f * m_player->getEnergy() / m_player->getMaxEnergy());
    if (energyWidth > 0) {
        // Color based on energy level
        if (m_player->getEnergy() > m_player->getMaxEnergy() * 0.6f) {
            SDL_SetRenderDrawColor(m_renderer, 0, 255, 0, 255); // Green
        } else if (m_player->getEnergy() > m_player->getMaxEnergy() * 0.3f) {
            SDL_SetRenderDrawColor(m_renderer, 255, 255, 0, 255); // Yellow
        } else {
            SDL_SetRenderDrawColor(m_renderer, 255, 0, 0, 255); // Red
        }
        SDL_Rect energyFill = {windowWidth - 208, windowHeight - 38, energyWidth, 26};
        SDL_RenderFillRect(m_renderer, &energyFill);
    }
    
    // Energy text
    if (m_subtitleFont) {
        std::string energyText = "E:" + std::to_string(m_player->getEnergy()) + "/" + std::to_string(m_player->getMaxEnergy());
        SDL_Color energyColor = {255, 255, 255, 255};
        SDL_Surface* energySurface = TTF_RenderText_Blended(m_subtitleFont, energyText.c_str(), energyColor);
        if (energySurface) {
            SDL_Texture* energyTexture = SDL_CreateTextureFromSurface(m_renderer, energySurface);
            if (energyTexture) {
                SDL_Rect energyTextRect = {windowWidth - 205, windowHeight - 36, energySurface->w, energySurface->h};
                SDL_RenderCopy(m_renderer, energyTexture, nullptr, &energyTextRect);
                SDL_DestroyTexture(energyTexture);
            }
            SDL_FreeSurface(energySurface);
        }
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
    if (m_stage01Music) {
        Mix_FreeMusic(m_stage01Music);
        m_stage01Music = nullptr;
    }
    if (m_stage02Music) {
        Mix_FreeMusic(m_stage02Music);
        m_stage02Music = nullptr;
    }
    if (m_boss01Music) {
        Mix_FreeMusic(m_boss01Music);
        m_boss01Music = nullptr;
    }
    if (m_boss02Music) {
        Mix_FreeMusic(m_boss02Music);
        m_boss02Music = nullptr;
    }
    if (m_boss03Music) {
        Mix_FreeMusic(m_boss03Music);
        m_boss03Music = nullptr;
    }
    if (m_boss04Music) {
        Mix_FreeMusic(m_boss04Music);
        m_boss04Music = nullptr;
    }
    
    Mix_CloseAudio();
    
    // Release textures
    if (m_backgroundTexture) {
        SDL_DestroyTexture(m_backgroundTexture);
        m_backgroundTexture = nullptr;
    }
    if (m_background01Texture) {
        SDL_DestroyTexture(m_background01Texture);
        m_background01Texture = nullptr;
    }
    if (m_background02Texture) {
        SDL_DestroyTexture(m_background02Texture);
        m_background02Texture = nullptr;
    }
    if (m_background03Texture) {
        SDL_DestroyTexture(m_background03Texture);
        m_background03Texture = nullptr;
    }
    if (m_background04Texture) {
        SDL_DestroyTexture(m_background04Texture);
        m_background04Texture = nullptr;
    }
    if (m_playerTexture) {
        SDL_DestroyTexture(m_playerTexture);
        m_playerTexture = nullptr;
    }
    if (m_shipStopTexture) {
        SDL_DestroyTexture(m_shipStopTexture);
        m_shipStopTexture = nullptr;
    }
    if (m_shipForwardTexture) {
        SDL_DestroyTexture(m_shipForwardTexture);
        m_shipForwardTexture = nullptr;
    }
    if (m_shipBackwardTexture) {
        SDL_DestroyTexture(m_shipBackwardTexture);
        m_shipBackwardTexture = nullptr;
    }
    if (m_shipLeftTexture) {
        SDL_DestroyTexture(m_shipLeftTexture);
        m_shipLeftTexture = nullptr;
    }
    if (m_shipRightTexture) {
        SDL_DestroyTexture(m_shipRightTexture);
        m_shipRightTexture = nullptr;
    }
    if (m_enemy01Texture) {
        SDL_DestroyTexture(m_enemy01Texture);
        m_enemy01Texture = nullptr;
    }
    if (m_enemy02Texture) {
        SDL_DestroyTexture(m_enemy02Texture);
        m_enemy02Texture = nullptr;
    }
    if (m_enemy03Texture) {
        SDL_DestroyTexture(m_enemy03Texture);
        m_enemy03Texture = nullptr;
    }
    if (m_enemy04Texture) {
        SDL_DestroyTexture(m_enemy04Texture);
        m_enemy04Texture = nullptr;
    }
    if (m_enemy05Texture) {
        SDL_DestroyTexture(m_enemy05Texture);
        m_enemy05Texture = nullptr;
    }
    if (m_enemyTexture) {
        m_enemyTexture = nullptr;  // Just nullify, already destroyed
    }
    if (m_boss01Texture) {
        SDL_DestroyTexture(m_boss01Texture);
        m_boss01Texture = nullptr;
    }
    if (m_boom01Texture) {
        SDL_DestroyTexture(m_boom01Texture);
        m_boom01Texture = nullptr;
    }
    if (m_boom02Texture) {
        SDL_DestroyTexture(m_boom02Texture);
        m_boom02Texture = nullptr;
    }
    if (m_boom03Texture) {
        SDL_DestroyTexture(m_boom03Texture);
        m_boom03Texture = nullptr;
    }
    if (m_boom04Texture) {
        SDL_DestroyTexture(m_boom04Texture);
        m_boom04Texture = nullptr;
    }
    if (m_boom05Texture) {
        SDL_DestroyTexture(m_boom05Texture);
        m_boom05Texture = nullptr;
    }
    if (m_boom06Texture) {
        SDL_DestroyTexture(m_boom06Texture);
        m_boom06Texture = nullptr;
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
    int windowWidth, windowHeight;
    SDL_GetWindowSize(m_window, &windowWidth, &windowHeight);
    
    // Determine which text to show based on timer
    const char* displayText = nullptr;
    if (m_stateTimer > 1.0f) {
        displayText = "GET READY";  // Show for first second
    } else {
        displayText = "GO";  // Show for last second
    }
    
    // Blinking effect: show/hide every 0.5 seconds
    float blinkPhase = fmod(m_stateTimer, 1.0f);  // Get fractional part of timer
    bool shouldShow = (static_cast<int>(blinkPhase * 2) % 2) == 0;
    
    if (shouldShow && m_titleFont) {
        SDL_Color textColor = {255, 255, 255, 255};  // White
        SDL_Surface* textSurface = TTF_RenderText_Blended(m_titleFont, displayText, textColor);
        if (textSurface) {
            SDL_Texture* textTexture = SDL_CreateTextureFromSurface(m_renderer, textSurface);
            if (textTexture) {
                SDL_Rect textRect = {
                    (windowWidth - textSurface->w) / 2,
                    (windowHeight - textSurface->h) / 2,
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

// Draw a single 7-segment digit
void Game::draw7SegmentDigit(int digit, int x, int y, int width, int height, SDL_Color color) {
    // 7-segment layout:
    //     A
    //   F   B
    //     G
    //   E   C
    //     D
    
    int segThickness = height / 8;
    int segWidth = width - segThickness * 2;
    int segHeight = (height - segThickness * 3) / 2;
    
    // Define which segments are lit for each digit (A, B, C, D, E, F, G)
    bool segments[10][7] = {
        {1,1,1,1,1,1,0}, // 0
        {0,1,1,0,0,0,0}, // 1
        {1,1,0,1,1,0,1}, // 2
        {1,1,1,1,0,0,1}, // 3
        {0,1,1,0,0,1,1}, // 4
        {1,0,1,1,0,1,1}, // 5
        {1,0,1,1,1,1,1}, // 6
        {1,1,1,0,0,0,0}, // 7
        {1,1,1,1,1,1,1}, // 8
        {1,1,1,1,0,1,1}  // 9
    };
    
    if (digit < 0 || digit > 9) return;
    
    SDL_SetRenderDrawColor(m_renderer, color.r, color.g, color.b, color.a);
    
    // Segment A (top)
    if (segments[digit][0]) {
        SDL_Rect segA = {x + segThickness, y, segWidth, segThickness};
        SDL_RenderFillRect(m_renderer, &segA);
    }
    
    // Segment B (top right)
    if (segments[digit][1]) {
        SDL_Rect segB = {x + width - segThickness, y + segThickness, segThickness, segHeight};
        SDL_RenderFillRect(m_renderer, &segB);
    }
    
    // Segment C (bottom right)
    if (segments[digit][2]) {
        SDL_Rect segC = {x + width - segThickness, y + height - segHeight - segThickness, segThickness, segHeight};
        SDL_RenderFillRect(m_renderer, &segC);
    }
    
    // Segment D (bottom)
    if (segments[digit][3]) {
        SDL_Rect segD = {x + segThickness, y + height - segThickness, segWidth, segThickness};
        SDL_RenderFillRect(m_renderer, &segD);
    }
    
    // Segment E (bottom left)
    if (segments[digit][4]) {
        SDL_Rect segE = {x, y + height - segHeight - segThickness, segThickness, segHeight};
        SDL_RenderFillRect(m_renderer, &segE);
    }
    
    // Segment F (top left)
    if (segments[digit][5]) {
        SDL_Rect segF = {x, y + segThickness, segThickness, segHeight};
        SDL_RenderFillRect(m_renderer, &segF);
    }
    
    // Segment G (middle)
    if (segments[digit][6]) {
        SDL_Rect segG = {x + segThickness, y + segThickness + segHeight, segWidth, segThickness};
        SDL_RenderFillRect(m_renderer, &segG);
    }
}

// Draw a multi-digit number using 7-segment display
void Game::draw7SegmentNumber(int number, int x, int y, int digitWidth, int digitHeight, int spacing, SDL_Color color) {
    std::string numStr = std::to_string(number);
    int currentX = x;
    
    for (char c : numStr) {
        int digit = c - '0';
        draw7SegmentDigit(digit, currentX, y, digitWidth, digitHeight, color);
        currentX += digitWidth + spacing;
    }
}

void Game::renderGameOver() {
    int windowWidth, windowHeight;
    SDL_GetWindowSize(m_window, &windowWidth, &windowHeight);
    
    // "GAME OVER" text
    if (m_uiFont) {
        SDL_Color gameOverColor = {255, 0, 0, 255};
        SDL_Surface* gameOverSurface = TTF_RenderText_Blended(m_uiFont, "GAME OVER", gameOverColor);
        if (gameOverSurface) {
            SDL_Texture* gameOverTexture = SDL_CreateTextureFromSurface(m_renderer, gameOverSurface);
            if (gameOverTexture) {
                SDL_Rect gameOverRect = {
                    (windowWidth - gameOverSurface->w) / 2,
                    100,
                    gameOverSurface->w,
                    gameOverSurface->h
                };
                SDL_RenderCopy(m_renderer, gameOverTexture, nullptr, &gameOverRect);
                SDL_DestroyTexture(gameOverTexture);
            }
            SDL_FreeSurface(gameOverSurface);
        }
    }
    
    // Current score with 7-segment display
    SDL_Color scoreColor = {255, 255, 255, 255};
    int scoreWidth = static_cast<int>(std::to_string(m_score).length()) * 35;
    draw7SegmentNumber(m_score, (windowWidth - scoreWidth) / 2, 180, 30, 45, 5, scoreColor);
    
    // High scores list
    if (m_subtitleFont) {
        int y = 280;
        for (size_t i = 0; i < m_highScores.size() && i < 10; i++) {
            // Rank number
            SDL_Color rankColor = {255, 215, 0, 255};
            std::string rankText = std::to_string(i + 1) + ".";
            SDL_Surface* rankSurface = TTF_RenderText_Blended(m_subtitleFont, rankText.c_str(), rankColor);
            if (rankSurface) {
                SDL_Texture* rankTexture = SDL_CreateTextureFromSurface(m_renderer, rankSurface);
                if (rankTexture) {
                    SDL_Rect rankRect = {windowWidth / 2 - 150, y, rankSurface->w, rankSurface->h};
                    SDL_RenderCopy(m_renderer, rankTexture, nullptr, &rankRect);
                    SDL_DestroyTexture(rankTexture);
                }
                SDL_FreeSurface(rankSurface);
            }
            
            // Score with 7-segment
            SDL_Color hsColor = {200, 200, 200, 255};
            draw7SegmentNumber(m_highScores[i], windowWidth / 2 - 50, y - 3, 15, 22, 3, hsColor);
            
            y += 30;
        }
    }
    
    // "CLICK TO RESTART" message
    if (m_subtitleFont) {
        SDL_Color restartColor = {0, 255, 0, 255};
        SDL_Surface* restartSurface = TTF_RenderText_Blended(m_subtitleFont, "CLICK TO RESTART", restartColor);
        if (restartSurface) {
            SDL_Texture* restartTexture = SDL_CreateTextureFromSurface(m_renderer, restartSurface);
            if (restartTexture) {
                SDL_Rect restartRect = {
                    (windowWidth - restartSurface->w) / 2,
                    windowHeight - 80,
                    restartSurface->w,
                    restartSurface->h
                };
                SDL_RenderCopy(m_renderer, restartTexture, nullptr, &restartRect);
                SDL_DestroyTexture(restartTexture);
            }
            SDL_FreeSurface(restartSurface);
        }
    }
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
    // Random power-up type with weighted probabilities
    int rand_val = rand() % 100;
    PowerUpType powerUpType;
    
    if (rand_val < 20) {
        // 20% - Speed
        powerUpType = PowerUpType::SPEED;
    } else if (rand_val < 40) {
        // 20% - Laser
        powerUpType = PowerUpType::LASER;
    } else if (rand_val < 60) {
        // 20% - Missile
        powerUpType = PowerUpType::MISSILE;
    } else if (rand_val < 70) {
        // 10% - Energy (random size)
        int energy_type = rand() % 3;
        if (energy_type == 0) {
            powerUpType = PowerUpType::ENERGY_SMALL;
        } else if (energy_type == 1) {
            powerUpType = PowerUpType::ENERGY_MEDIUM;
        } else {
            powerUpType = PowerUpType::ENERGY_LARGE;
        }
    } else if (rand_val < 80) {
        // 10% - Bonus
        powerUpType = PowerUpType::BONUS;
    } else if (rand_val < 85) {
        // 5% - 1UP
        powerUpType = PowerUpType::ONE_UP;
    } else if (rand_val < 90) {
        // 5% - Voltage
        powerUpType = PowerUpType::VOLTAGE;
    } else if (rand_val < 95) {
        // 5% - Keep items (random)
        int keep_type = rand() % 3;
        if (keep_type == 0) {
            powerUpType = PowerUpType::KEEP_SPEED;
        } else if (keep_type == 1) {
            powerUpType = PowerUpType::KEEP_LASER;
        } else {
            powerUpType = PowerUpType::KEEP_MISSILE;
        }
    } else {
        // 5% - Penalty items (careful!)
        int penalty_type = rand() % 4;
        if (penalty_type == 0) {
            powerUpType = PowerUpType::SPEED_DOWN;
        } else if (penalty_type == 1) {
            powerUpType = PowerUpType::LASER_DOWN;
        } else if (penalty_type == 2) {
            powerUpType = PowerUpType::MISSILE_DOWN;
        } else {
            powerUpType = PowerUpType::ENERGY_DOWN;
        }
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
                    m_player->collectSpeed();
                    SDL_Log("INFO: Speed item collected! (%d/3)", m_player->getSpeedCount());
                    break;
                case PowerUpType::LASER:
                    m_player->collectLaser();
                    SDL_Log("INFO: Laser item collected! (%d/3)", m_player->getLaserCount());
                    break;
                case PowerUpType::MISSILE:
                    m_player->collectMissile();
                    SDL_Log("INFO: Missile item collected! (%d/3)", m_player->getMissileCount());
                    break;
                case PowerUpType::ENERGY_SMALL:
                    m_player->addEnergy(1);
                    SDL_Log("INFO: +1 Energy");
                    break;
                case PowerUpType::ENERGY_MEDIUM:
                    m_player->addEnergy(4);
                    SDL_Log("INFO: +4 Energy");
                    break;
                case PowerUpType::ENERGY_LARGE:
                    m_player->addEnergy(8);
                    SDL_Log("INFO: +8 Energy");
                    break;
                case PowerUpType::BONUS:
                    m_score += 2000;
                    SDL_Log("INFO: Bonus +2000!");
                    break;
                case PowerUpType::KEEP_SPEED:
                    m_player->setKeepSpeed(true);
                    SDL_Log("INFO: Speed Keep activated!");
                    break;
                case PowerUpType::KEEP_LASER:
                    m_player->setKeepLaser(true);
                    SDL_Log("INFO: Laser Keep activated!");
                    break;
                case PowerUpType::KEEP_MISSILE:
                    m_player->setKeepMissile(true);
                    SDL_Log("INFO: Missile Keep activated!");
                    break;
                case PowerUpType::ONE_UP:
                    m_lives++;
                    SDL_Log("INFO: 1UP! Lives: %d", m_lives);
                    break;
                case PowerUpType::VOLTAGE:
                    m_player->increaseMaxEnergy();
                    SDL_Log("INFO: Max Energy increased to %d", m_player->getMaxEnergy());
                    break;
                case PowerUpType::SPEED_DOWN:
                    m_player->downgradeSpeed();
                    SDL_Log("WARNING: Speed Down!");
                    break;
                case PowerUpType::LASER_DOWN:
                    m_player->downgradeLaser();
                    SDL_Log("WARNING: Laser Down!");
                    break;
                case PowerUpType::MISSILE_DOWN:
                    m_player->downgradeMissile();
                    SDL_Log("WARNING: Missile Down!");
                    break;
                case PowerUpType::ENERGY_DOWN:
                    m_player->removeEnergy(4);
                    SDL_Log("WARNING: -4 Energy!");
                    break;
                case PowerUpType::ARMOR_HEAD:
                case PowerUpType::ARMOR_LEFT:
                case PowerUpType::ARMOR_RIGHT:
                    SDL_Log("INFO: Armor part collected (Phase 3 feature)");
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
void Game::checkItemBoxCollection() {
    for (auto boxIt = m_itemBoxes.begin(); boxIt != m_itemBoxes.end();) {
        // Only collect revealed boxes
        if (!(*boxIt)->isRevealed()) {
            ++boxIt;
            continue;
        }
        
        // Check collision with player
        if (m_player->getX() < (*boxIt)->getX() + (*boxIt)->getWidth() &&
            m_player->getX() + m_player->getWidth() > (*boxIt)->getX() &&
            m_player->getY() < (*boxIt)->getY() + (*boxIt)->getHeight() &&
            m_player->getY() + m_player->getHeight() > (*boxIt)->getY()) {
            
            // Apply item effect based on revealed state
            ItemBox::BoxState state = (*boxIt)->getState();
            
            if (state == ItemBox::BoxState::REVEALED_S) {
                // Speed up
                m_player->collectSpeed();
                SDL_Log("INFO: Speed collected from item box!");
            } else if (state == ItemBox::BoxState::REVEALED_L) {
                // Laser up
                m_player->collectLaser();
                SDL_Log("INFO: Laser collected from item box!");
            } else if (state == ItemBox::BoxState::REVEALED_M) {
                // Missile up
                m_player->collectMissile();
                SDL_Log("INFO: Missile collected from item box!");
            }
            
            // Play sound
            if (m_shootSound) {
                Mix_PlayChannel(-1, m_shootSound, 0);
            }
            
            boxIt = m_itemBoxes.erase(boxIt);
        } else {
            ++boxIt;
        }
    }
}

void Game::checkMissileItemBoxCollision() {
    for (auto bulletIt = m_bullets.begin(); bulletIt != m_bullets.end();) {
        bool bulletRemoved = false;
        
        // Only missiles can reveal item boxes
        if ((*bulletIt)->getType() == Bullet::BulletType::MISSILE) {
            for (auto& box : m_itemBoxes) {
                // Only hidden boxes can be revealed
                if (box->getState() == ItemBox::BoxState::HIDDEN) {
                    // Check collision
                    if ((*bulletIt)->getX() < box->getX() + box->getWidth() &&
                        (*bulletIt)->getX() + (*bulletIt)->getWidth() > box->getX() &&
                        (*bulletIt)->getY() < box->getY() + box->getHeight() &&
                        (*bulletIt)->getY() + (*bulletIt)->getHeight() > box->getY()) {
                        
                        // Reveal the box (open it)
                        box->reveal();
                        SDL_Log("INFO: Item box opened!");
                        
                        // Play explosion sound
                        if (m_explosionSound) {
                            Mix_PlayChannel(-1, m_explosionSound, 0);
                        }
                        
                        bulletIt = m_bullets.erase(bulletIt);
                        bulletRemoved = true;
                        break;
                    }
                }
            }
        }
        
        if (!bulletRemoved) {
            ++bulletIt;
        }
    }
}

void Game::spawnItemBoxes() {
    int windowWidth, windowHeight;
    SDL_GetWindowSize(m_window, &windowWidth, &windowHeight);
    
    // Spawn 3-5 random item boxes at the start
    int boxCount = 3 + (rand() % 3);  // 3-5 boxes
    
    for (int i = 0; i < boxCount; i++) {
        float boxX = 50.0f + (rand() % (windowWidth - 100));
        float boxY = -200.0f - (i * 150.0f);  // Stagger vertically
        
        m_itemBoxes.push_back(std::make_unique<ItemBox>(boxX, boxY));
    }
    
    SDL_Log("INFO: Spawned %d item boxes", boxCount);
}