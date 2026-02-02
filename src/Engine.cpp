#include "Engine.h"
#include <stdexcept>

/// @brief Constructor
Engine::Engine()
        : mWindow(sf::VideoMode(sf::Vector2u(Config::WINDOW_WIDTH, Config::WINDOW_HEIGHT)), "RetroRush"),
          mCamera(sf::FloatRect(sf::Vector2f(0.f, 0.f), sf::Vector2f(Config::CAMERA_WIDTH, Config::CAMERA_HEIGHT))),
          mTimePerFrame(sf::seconds(Config::TIME_PER_FRAME)) {
    /// Set frame rate limit
    mWindow.setFramerateLimit(Config::FPS);

    /// Load textures
    std::string circuitPath = Config::TEXTURES_PATH + "circuit.png";
    std::string voiturePath = Config::TEXTURES_PATH + "voiture.png";
    if (!mAssetsManager.loadTexture("circuit", circuitPath) ||
        !mAssetsManager.loadTexture("voiture", voiturePath)) {
        throw std::runtime_error("Failed to load textures");
    }

    /// Initialize world
    mWorld = std::make_unique<World>(mWindow, mAssetsManager);
    mCamera.setViewport(sf::FloatRect({0.f, 0.f}, {1.f, 1.f}));

    /// Load font
    if (!mAssetsManager.loadFont("arial", Config::FONTS_PATH + "arial.ttf")) {
        throw std::runtime_error("Failed to load font arial");
    }

    /// Initialize UI and managers
    sf::Font& font = mAssetsManager.getFont("arial");
    mMenu = std::make_unique<Menu>(font);
    mHud = std::make_unique<HUD>(font);
    mCameraManager = std::make_unique<Camera>(Config::CAMERA_WIDTH, Config::CAMERA_HEIGHT);
    mGameManager = std::make_unique<GameManager>();
}

/// @brief Run game loop
void Engine::run() {
    sf::Clock clock;
    sf::Time timeSinceLastUpdate = sf::Time::Zero;

    /// Main game loop
    while (mWindow.isOpen()) {
        sf::Time deltaTime = clock.restart();
        timeSinceLastUpdate += deltaTime;

        /// Update at fixed intervals
        while (timeSinceLastUpdate > mTimePerFrame) {
            timeSinceLastUpdate -= mTimePerFrame;
            processEvents();
            update(mTimePerFrame);
        }

        render();
    }
}

/// @brief Process window events
void Engine::processEvents() {
    while (auto eventOpt = mWindow.pollEvent()) {
        const sf::Event& event = *eventOpt;
        if (event.is<sf::Event::Closed>()) {
            mWindow.close();
        }
        else if (event.is<sf::Event::MouseButtonPressed>() &&
                 (mGameManager->isInMenu() || mGameManager->isFinished())) {
            sf::Vector2f mousePos = mWindow.mapPixelToCoords(sf::Mouse::getPosition(mWindow));
            if (mMenu->isButtonClicked(mousePos)) {
                mGameManager->reset();
                mGameManager->startCountdown();
                mWorld->reset();
            }
        }
    }
}

/// @brief Update game state
/// @param deltaTime Time since last update
void Engine::update(sf::Time deltaTime) {
    bool justStarted = mGameManager->justStartedRace();

    /// Update game manager
    mGameManager->update();

    /// Start player clock on race start
    if (justStarted) {
        mWorld->getPlayer().startClock();
    }

    /// Update world and camera during gameplay
    if (mGameManager->isPlaying()) {
        mWorld->update(deltaTime, mCamera);
        mCameraManager->update(mCamera, mWorld->getCar().getPosition(), mWorld->getTrackBounds().size);

        /// Handle lap completion
        if (mWorld->isLapComplete() && mWorld->getLapCount() >= 1) {
            float raceTime = mGameManager->getRaceTime();
            mGameManager->markLapFinished(raceTime);
            mMenu->setResultText(mGameManager->getResultText());
            mMenu->setButtonText("Rejouer");

            /// Update ghost and HUD with times
            mWorld->getGhost().submitTime(mWorld->getPlayer().getElapsedTime());
            mHud->setBestTimes(mWorld->getGhost().getBestTimes());
        }
    }

    /// Update HUD display
    float speed = mWorld->getCar().getSpeed() * 3.6f; ///< Convert to km/h
    float time = mGameManager->getRaceTime();
    int countdown = mGameManager->isCountdown() ? mGameManager->getCountdownValue() : -2;
    mHud->update(speed, time, countdown, mWindow.getSize());
    mHud->setBestTimes(mWorld->getGhost().getBestTimes());
}

/// @brief Render game
void Engine::render() {
    mWindow.clear(sf::Color(50, 50, 50));

    if (mGameManager->isInMenu() || mGameManager->isFinished()) {
        /// Render menu
        mWindow.setView(mWindow.getDefaultView());
        mMenu->render(mWindow, mGameManager->isFinished());
    } else {
        /// Render world and HUD
        mWindow.setView(mCamera);
        mWorld->render(mGameManager->isPlaying());
        mWindow.setView(mWindow.getDefaultView());
        mHud->render(mWindow);
    }

    mWindow.display();
}