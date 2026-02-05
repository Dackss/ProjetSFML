#ifndef ENGINE_H
#define ENGINE_H

#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <memory>
#include "World.h"
#include "AssetsManager.h"
#include "Menu.h"
#include "Hud.h"
#include "Camera.h"
#include "GameManager.h"
#include "Config.h"

/// @brief Main game engine class
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
    void render(float alpha = 1.0f);

    /// @brief Toggle between Fullscreen and Windowed mode
    void toggleFullscreen();

private:
    // L'ordre de déclaration est important pour l'initialisation
    sf::ContextSettings mContextSettings;
    sf::RenderWindow mWindow;
    sf::View mCamera;
    sf::Time mTimePerFrame;

    AssetsManager mAssetsManager;
    std::unique_ptr<World> mWorld;
    std::unique_ptr<Menu> mMenu;
    std::unique_ptr<HUD> mHud;
    std::unique_ptr<Camera> mCameraManager;
    std::unique_ptr<GameManager> mGameManager;

    bool mIsFullscreen;
    bool mHasFocus;
};

#endif // ENGINE_H