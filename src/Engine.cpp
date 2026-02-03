#include "Engine.h"
#include "ScoreManager.h"
#include <SFML/Window/Joystick.hpp>
#include <stdexcept>

/// @brief Constructor
Engine::Engine()
    // Initialisation en Plein Écran par défaut pour l'Arcade
    : mWindow(sf::VideoMode::getDesktopMode(), "RetroRush", sf::Style::Default, sf::State::Fullscreen),
      mCamera(sf::FloatRect({0.f, 0.f}, {Config::CAMERA_WIDTH, Config::CAMERA_HEIGHT})),
      mTimePerFrame(sf::seconds(Config::TIME_PER_FRAME)),
      mIsFullscreen(true)
{
    // mWindow.setFramerateLimit(static_cast<unsigned int>(Config::FPS)); // Désactivé
    mWindow.setVerticalSyncEnabled(true);

    // Masquer le curseur pour l'immersion
    mWindow.setMouseCursorVisible(false);

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
    sf::Texture& bgTexture = mAssetsManager.getTexture("circuit");

    // Passage de la texture au menu
    mMenu = std::make_unique<Menu>(font, bgTexture);

    mHud = std::make_unique<HUD>(font);
    mCameraManager = std::make_unique<Camera>(Config::CAMERA_WIDTH, Config::CAMERA_HEIGHT);
    mGameManager = std::make_unique<GameManager>();

    mCameraManager->update(mCamera, mWorld->getCar().getPosition(), mWorld->getTrackBounds().size);
}

/// @brief Run game loop
void Engine::run() {
    sf::Clock clock;
    sf::Time timeSinceLastUpdate = sf::Time::Zero;

    /// Main game loop
    while (mWindow.isOpen()) {
        sf::Time deltaTime = clock.restart();
        timeSinceLastUpdate += deltaTime;

        /// Update at fixed intervals (Physique stable)
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
        else if (const auto* keyEvent = event.getIf<sf::Event::KeyPressed>()) {
            if (keyEvent->code == sf::Keyboard::Key::Escape) {
                mWindow.close();
            }
            // GESTION F11 : Basculement Plein Écran / Fenêtré
            else if (keyEvent->code == sf::Keyboard::Key::F11) {
                mIsFullscreen = !mIsFullscreen;

                auto state = mIsFullscreen ? sf::State::Fullscreen : sf::State::Windowed;
                mWindow.create(sf::VideoMode::getDesktopMode(), "RetroRush", sf::Style::Default, state);

                // Important : Réappliquer les paramètres après recréation de la fenêtre
                mWindow.setVerticalSyncEnabled(true);
                mWindow.setMouseCursorVisible(!mIsFullscreen);
            }
        }

        // GESTION MENU (Clavier / Manette)
        if (mGameManager->isInMenu() || mGameManager->isFinished()) {
            bool startRequested = false;

            if (const auto* keyEvent = event.getIf<sf::Event::KeyPressed>()) {
                if (keyEvent->code == sf::Keyboard::Key::Enter || keyEvent->code == sf::Keyboard::Key::Space) {
                    startRequested = true;
                }
            }
            else if (const auto* joyEvent = event.getIf<sf::Event::JoystickButtonPressed>()) {
                if (joyEvent->button == 0 || joyEvent->button == 7) {
                    startRequested = true;
                }
            }

            if (startRequested) {
                // Si on relance après une fin de course, on sauvegarde le score
                if (mGameManager->isFinished()) {
                    ScoreManager::saveTime(mGameManager->getRaceTime());
                    mMenu->updateHighScores(); // Rafraîchir l'affichage des scores
                }

                mGameManager->reset();
                mGameManager->startCountdown();
                mWorld->reset();

                // Recaler la caméra après le reset
                mCameraManager->update(mCamera, mWorld->getCar().getPosition(), mWorld->getTrackBounds().size);
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