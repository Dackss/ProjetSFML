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

class Engine {
public:
    Engine();
    void run();

private:
    void processEvents();
    void update(sf::Time deltaTime);
    void render(float alpha);

    // Nouvelle fonction pour gérer proprement la création/bascule
    void recreateWindow();
    void toggleFullscreen();

private:
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