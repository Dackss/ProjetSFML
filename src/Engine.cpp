#include "Engine.h"
#include "ScoreManager.h"
#include <SFML/Window/Joystick.hpp>
#include <stdexcept>
#include <cmath>

/// @brief Helper to maintain aspect ratio
static void adjustView(const sf::Vector2u& windowSize, sf::View& view, float targetRatio) {
    float windowRatio = static_cast<float>(windowSize.x) / static_cast<float>(windowSize.y);

    if (windowRatio > targetRatio) {
        // Fenêtre plus large que le jeu (Bandes noires sur les côtés)
        float scaleFactor = targetRatio / windowRatio;
        view.setViewport(sf::FloatRect({(1.f - scaleFactor) / 2.f, 0.f}, {scaleFactor, 1.f}));
    } else {
        // Fenêtre plus haute que le jeu (Bandes noires en haut/bas)
        float scaleFactor = windowRatio / targetRatio;
        view.setViewport(sf::FloatRect({0.f, (1.f - scaleFactor) / 2.f}, {1.f, scaleFactor}));
    }
}

Engine::Engine()
    // On utilise getDesktopMode() qui cible l'écran principal par défaut
    : mWindow(sf::VideoMode::getDesktopMode(), "RetroRush", sf::Style::Default, sf::State::Fullscreen),
      mCamera(sf::FloatRect({0.f, 0.f}, {Config::CAMERA_WIDTH, Config::CAMERA_HEIGHT})),
      mTimePerFrame(sf::seconds(Config::TIME_PER_FRAME)),
      mIsFullscreen(true)
{
    mWindow.setVerticalSyncEnabled(true);
    mWindow.setMouseCursorVisible(false);

    // Ajustement immédiat du ratio pour l'écran actuel
    adjustView(mWindow.getSize(), mCamera, Config::CAMERA_WIDTH / Config::CAMERA_HEIGHT);

    /// Load textures
    std::string circuitPath = Config::TEXTURES_PATH + "circuit.png";
    std::string voiturePath = Config::TEXTURES_PATH + "voiture.png";
    if (!mAssetsManager.loadTexture("circuit", circuitPath) ||
        !mAssetsManager.loadTexture("voiture", voiturePath)) {
        throw std::runtime_error("Failed to load textures");
    }

    /// Initialize world
    mWorld = std::make_unique<World>(mWindow, mAssetsManager);

    /// Load font
    if (!mAssetsManager.loadFont("arial", Config::FONTS_PATH + "arial.ttf")) {
        throw std::runtime_error("Failed to load font arial");
    }

    /// Initialize UI and managers
    sf::Font& font = mAssetsManager.getFont("arial");
    sf::Texture& bgTexture = mAssetsManager.getTexture("circuit");

    mMenu = std::make_unique<Menu>(font, bgTexture);
    mHud = std::make_unique<HUD>(font);
    mCameraManager = std::make_unique<Camera>(Config::CAMERA_WIDTH, Config::CAMERA_HEIGHT);
    mGameManager = std::make_unique<GameManager>();

    mCameraManager->update(mCamera, mWorld->getCar().getPosition(), mWorld->getTrackBounds().size);
}

void Engine::run() {
    sf::Clock clock;
    sf::Time timeSinceLastUpdate = sf::Time::Zero;

    while (mWindow.isOpen()) {
        sf::Time deltaTime = clock.restart();
        timeSinceLastUpdate += deltaTime;

        while (timeSinceLastUpdate > mTimePerFrame) {
            timeSinceLastUpdate -= mTimePerFrame;
            processEvents();
            update(mTimePerFrame);
        }
        float alpha = timeSinceLastUpdate.asSeconds() / mTimePerFrame.asSeconds();

        render(alpha);
    }
}

void Engine::processEvents() {
    while (auto eventOpt = mWindow.pollEvent()) {
        const sf::Event& event = *eventOpt;

        if (event.is<sf::Event::Closed>()) {
            mWindow.close();
        }
        else if (const auto* resizeEvent = event.getIf<sf::Event::Resized>()) {
            // Gestion du redimensionnement (garde le ratio)
            adjustView(resizeEvent->size, mCamera, Config::CAMERA_WIDTH / Config::CAMERA_HEIGHT);
        }
        else if (const auto* keyEvent = event.getIf<sf::Event::KeyPressed>()) {
            if (keyEvent->code == sf::Keyboard::Key::Escape) {
                mWindow.close();
            }
            // GESTION F11 : Basculement Plein Écran / Fenêtré
            else if (keyEvent->code == sf::Keyboard::Key::F11) {
                mIsFullscreen = !mIsFullscreen;

                auto state = mIsFullscreen ? sf::State::Fullscreen : sf::State::Windowed;
                // Si on passe en fenêtré, on met une taille standard, sinon DesktopMode
                sf::VideoMode mode = mIsFullscreen ? sf::VideoMode::getDesktopMode() : sf::VideoMode({1280, 720});

                mWindow.create(mode, "RetroRush", sf::Style::Default, state);

                // Réappliquer les paramètres
                mWindow.setVerticalSyncEnabled(true);
                mWindow.setMouseCursorVisible(!mIsFullscreen);

                // Réajuster la vue après recréation
                adjustView(mWindow.getSize(), mCamera, Config::CAMERA_WIDTH / Config::CAMERA_HEIGHT);
            }
        }

        // GESTION MENU
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
                if (mGameManager->isFinished()) {
                    ScoreManager::saveTime(mGameManager->getRaceTime());
                    mMenu->updateHighScores();
                }

                mGameManager->reset();
                mGameManager->startCountdown();
                mWorld->reset();
                mCameraManager->update(mCamera, mWorld->getCar().getPosition(), mWorld->getTrackBounds().size);
            }
        }
    }
}

void Engine::update(sf::Time deltaTime) {
    bool justStarted = mGameManager->justStartedRace();
    mGameManager->update();

    if (justStarted) {
        mWorld->getPlayer().startClock();
    }

    if (mGameManager->isPlaying()) {
        mWorld->update(deltaTime, mCamera);
        mCameraManager->update(mCamera, mWorld->getCar().getPosition(), mWorld->getTrackBounds().size);

        if (mWorld->isLapComplete() && mWorld->getLapCount() >= 1) {
            float raceTime = mGameManager->getRaceTime();
            mGameManager->markLapFinished(raceTime);
            mMenu->setResultText(mGameManager->getResultText());
            mWorld->getGhost().submitTime(mWorld->getPlayer().getElapsedTime());
            mHud->setBestTimes(mWorld->getGhost().getBestTimes());
        }
    }

    float speed = mWorld->getCar().getSpeed() * 3.6f;
    float time = mGameManager->getRaceTime();
    int countdown = mGameManager->isCountdown() ? mGameManager->getCountdownValue() : -2;
    mHud->update(speed, time, countdown, mWindow.getSize());
    mHud->setBestTimes(mWorld->getGhost().getBestTimes());
}

void Engine::render(float alpha) {
    mWindow.clear(sf::Color(20, 20, 20)); // Fond légèrement gris pour les bandes noires

    if (mGameManager->isInMenu() || mGameManager->isFinished()) {
        mWindow.setView(mWindow.getDefaultView());
        mMenu->render(mWindow, mGameManager->isFinished());
    } else {
        if (mGameManager->isPlaying()) {
            sf::Vector2f interpolatedCarPos = mWorld->getCar().getInterpolatedPosition(alpha);
            mCameraManager->update(mCamera, interpolatedCarPos, mWorld->getTrackBounds().size);
        }

        // On applique la caméra avec le viewport ajusté (Letterbox)
        mWindow.setView(mCamera);
        mWorld->render(mGameManager->isPlaying(), alpha);

        mWindow.setView(mWindow.getDefaultView());
        mHud->render(mWindow);
    }

    mWindow.display();
}