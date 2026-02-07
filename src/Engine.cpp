#include "Engine.h"
#include "ScoreManager.h"
#include <SFML/Window/Joystick.hpp>
#include <stdexcept>
#include <cmath>
#include <iostream>

// --- FONCTION UTILITAIRE ---

// Ajuste la vue pour garder le ratio d'aspect (bandes noires si nécessaire)
static void adjustView(const sf::Vector2u& windowSize, sf::View& view, float targetRatio) {
    float windowRatio = static_cast<float>(windowSize.x) / static_cast<float>(windowSize.y);

    if (windowRatio > targetRatio) {
        // Écran plus large que le jeu
        float scaleFactor = targetRatio / windowRatio;
        view.setViewport(sf::FloatRect({(1.f - scaleFactor) / 2.f, 0.f}, {scaleFactor, 1.f}));
    } else {
        // Écran plus haut que le jeu
        float scaleFactor = windowRatio / targetRatio;
        view.setViewport(sf::FloatRect({0.f, (1.f - scaleFactor) / 2.f}, {1.f, scaleFactor}));
    }
}

// --- CLASSE ENGINE ---

Engine::Engine()
    : mCamera(sf::FloatRect({0.f, 0.f}, {Config::CAMERA_WIDTH, Config::CAMERA_HEIGHT})),
      mTimePerFrame(sf::seconds(Config::TIME_PER_FRAME)),
      mIsFullscreen(true),
      mHasFocus(true)
{
    mContextSettings.antiAliasingLevel = Config::ANTIALIASING_LEVEL;

    // --- CRÉATION DE LA FENÊTRE (LOGIQUE FIX LAPTOP/WSL) ---
    sf::VideoMode mode;
    std::uint32_t style;
    sf::State state;

    if (mIsFullscreen && Config::USE_BORDERLESS_FULLSCREEN) {
        std::cout << "[VIDEO] Mode: Borderless Fullscreen (Optimisé WSL)" << std::endl;
        mode = sf::VideoMode::getDesktopMode();
        style = sf::Style::None;
        state = sf::State::Windowed;
    } else if (mIsFullscreen) {
        // Fallback classique
        mode = sf::VideoMode::getDesktopMode();
        style = sf::Style::Default;
        state = sf::State::Fullscreen;
    } else {
        // CORRECTION ICI : Ajout des accolades {} pour SFML 3
        mode = sf::VideoMode({static_cast<unsigned int>(Config::WINDOW_WIDTH), static_cast<unsigned int>(Config::WINDOW_HEIGHT)});
        style = sf::Style::Default;
        state = sf::State::Windowed;
    }

    mWindow.create(mode, "RetroRush", style, state, mContextSettings);

    // Positionnement en 0,0 si Borderless
    if (mIsFullscreen && Config::USE_BORDERLESS_FULLSCREEN) {
        mWindow.setPosition({0, 0});
    }

    // --- REGLAGES DE FLUIDITÉ ---
    mWindow.setVerticalSyncEnabled(Config::ENABLE_VSYNC);
    mWindow.setFramerateLimit(Config::FRAME_LIMIT);
    mWindow.setMouseCursorVisible(false);

    // Ajustement de la vue (caméra)
    adjustView(mWindow.getSize(), mCamera, Config::CAMERA_WIDTH / Config::CAMERA_HEIGHT);

    // --- Chargement des Textures ---
    unsigned int maxTextureSize = sf::Texture::getMaximumSize();
    printf("GPU Max Texture Size: %u px\n", maxTextureSize);

    std::string circuitFile;
    if (maxTextureSize <= Config::TEXTURE_LIMIT_THRESHOLD) {
        mAssetsManager.setUseSDAssets(true);
        circuitFile = Config::FILE_CIRCUIT_SD;
    } else {
        mAssetsManager.setUseSDAssets(false);
        circuitFile = Config::FILE_CIRCUIT_HD;
    }

    std::string circuitPath = Config::TEXTURES_PATH + circuitFile;
    std::string voiturePath = Config::TEXTURES_PATH + "voiture.png";

    if (!mAssetsManager.loadTexture("circuit", circuitPath) ||
        !mAssetsManager.loadTexture("voiture", voiturePath)) {
        throw std::runtime_error("Failed to load textures");
    }

    mWorld = std::make_unique<World>(mWindow, mAssetsManager);

    if (!mAssetsManager.loadFont("arial", Config::FONTS_PATH + "arial.ttf")) {
        throw std::runtime_error("Failed to load font arial");
    }

    sf::Font& font = mAssetsManager.getFont("arial");
    sf::Texture& bgTexture = mAssetsManager.getTexture("circuit");

    mMenu = std::make_unique<Menu>(font, bgTexture);
    mHud = std::make_unique<HUD>(font);
    mCameraManager = std::make_unique<Camera>(Config::CAMERA_WIDTH, Config::CAMERA_HEIGHT);
    mGameManager = std::make_unique<GameManager>();

    mCameraManager->update(mCamera, mWorld->getCar().getPosition(), mWorld->getTrackBounds().size);
    mHud->setBestTimes(mWorld->getGhost().getBestTimes());
}

void Engine::run() {
    sf::Clock clock;
    sf::Clock fpsClock;
    int frameCount = 0;
    sf::Time timeSinceLastUpdate = sf::Time::Zero;

    while (mWindow.isOpen()) {
        processEvents();

        sf::Time deltaTime = clock.restart();
        timeSinceLastUpdate += deltaTime;

        if (timeSinceLastUpdate > mTimePerFrame * 5.0f) {
            timeSinceLastUpdate = mTimePerFrame * 5.0f;
        }

        while (timeSinceLastUpdate > mTimePerFrame) {
            timeSinceLastUpdate -= mTimePerFrame;
            if (mHasFocus) {
                update(mTimePerFrame);
            }
        }

        frameCount++;
        if (fpsClock.getElapsedTime().asSeconds() >= 1.f) {
            float currentFps = static_cast<float>(frameCount) / fpsClock.restart().asSeconds();
            mHud->updateFPS(currentFps, mWindow.getSize());
            frameCount = 0;
        }

        float alpha = timeSinceLastUpdate.asSeconds() / mTimePerFrame.asSeconds();
        render(alpha);
    }
}

void Engine::toggleFullscreen() {
    mIsFullscreen = !mIsFullscreen;

    sf::VideoMode mode;
    std::uint32_t style;
    sf::State state;

    if (mIsFullscreen) {
        if (Config::USE_BORDERLESS_FULLSCREEN) {
            // MODE BORDERLESS
            mode = sf::VideoMode::getDesktopMode();
            style = sf::Style::None;
            state = sf::State::Windowed;
        } else {
            // MODE EXCLUSIF
            mode = sf::VideoMode::getDesktopMode();
            style = sf::Style::Default;
            state = sf::State::Fullscreen;
        }
    } else {
        // MODE FENÊTRE : Correction accolades ici aussi par sécurité
        mode = sf::VideoMode({static_cast<unsigned int>(Config::WINDOW_WIDTH), static_cast<unsigned int>(Config::WINDOW_HEIGHT)});
        style = sf::Style::Default;
        state = sf::State::Windowed;
    }

    mWindow.create(mode, "RetroRush", style, state, mContextSettings);

    if (mIsFullscreen && Config::USE_BORDERLESS_FULLSCREEN) {
        mWindow.setPosition({0, 0});
    } else if (!mIsFullscreen) {
        auto desktop = sf::VideoMode::getDesktopMode();
        sf::Vector2i position(
            (desktop.size.x - mode.size.x) / 2,
            (desktop.size.y - mode.size.y) / 2
        );
        mWindow.setPosition(position);
    }

    mWindow.setVerticalSyncEnabled(Config::ENABLE_VSYNC);
    mWindow.setFramerateLimit(Config::FRAME_LIMIT);
    mWindow.setMouseCursorVisible(!mIsFullscreen);

    adjustView(mWindow.getSize(), mCamera, Config::CAMERA_WIDTH / Config::CAMERA_HEIGHT);
}

void Engine::processEvents() {
    while (auto eventOpt = mWindow.pollEvent()) {
        const sf::Event& event = *eventOpt;

        if (event.is<sf::Event::Closed>()) {
            mWindow.close();
        }
        else if (const auto* resizeEvent = event.getIf<sf::Event::Resized>()) {
            adjustView(resizeEvent->size, mCamera, Config::CAMERA_WIDTH / Config::CAMERA_HEIGHT);
        }
        else if (event.is<sf::Event::FocusLost>()) {
            mHasFocus = false;
        }
        else if (event.is<sf::Event::FocusGained>()) {
            mHasFocus = true;
        }
        else if (const auto* keyEvent = event.getIf<sf::Event::KeyPressed>()) {
            if (keyEvent->code == sf::Keyboard::Key::Escape) {
                mWindow.close();
            }
            else if (keyEvent->code == sf::Keyboard::Key::F11) {
                toggleFullscreen();
            }
        }

        if (!mHasFocus) continue;

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

    if (mGameManager->isPlaying() && !mGameManager->isTimerRunning()) {
        if (mWorld->isOnStartLine()) {
            mGameManager->startTimer();
        }
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
}

void Engine::render(float alpha) {
    mWindow.clear(sf::Color(20, 20, 20));

    if (mGameManager->isInMenu() || mGameManager->isFinished()) {
        mWindow.setView(mWindow.getDefaultView());
        mMenu->render(mWindow, mGameManager->isFinished());
    } else {
        if (mGameManager->isPlaying()) {
            sf::Vector2f interpolatedCarPos = mWorld->getCar().getInterpolatedPosition(alpha);
            mCameraManager->update(mCamera, interpolatedCarPos, mWorld->getTrackBounds().size);
        }

        mWindow.setView(mCamera);
        mWorld->render(mGameManager->isPlaying(), alpha);

        mWindow.setView(mWindow.getDefaultView());
        mHud->render(mWindow);
    }

    mWindow.display();
}