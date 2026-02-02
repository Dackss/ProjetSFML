#ifndef ENGINE_H
#define ENGINE_H

#include "World.h"
#include "AssetsManager.h"
#include "Menu.h"
#include "HUD.h"
#include "Camera.h"
#include "Config.h"
#include "GameManager.h"

/// @brief Main game engine
class Engine {
public:
    /// @brief Constructor
    Engine();

    /// @brief Run game loop
    void run();

private:
    /// @brief Process window events
    void processEvents();

    /// @brief Update game state
    /// @param deltaTime Time since last update
    void update(sf::Time deltaTime);

    /// @brief Render game
    void render();

    sf::RenderWindow mWindow;           ///< Main window
    sf::View mCamera;                  ///< Camera view
    sf::Time mTimePerFrame;            ///< Fixed time per frame
    AssetsManager mAssetsManager;      ///< Resource manager
    std::unique_ptr<World> mWorld;     ///< Game world
    std::unique_ptr<Menu> mMenu;       ///< Menu UI
    std::unique_ptr<HUD> mHud;         ///< HUD UI
    std::unique_ptr<Camera> mCameraManager; ///< Camera controller
    std::unique_ptr<GameManager> mGameManager; ///< Game state manager
};

#endif // ENGINE_H