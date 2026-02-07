#include "Engine.h"
#include "Config.h"
#include "ScoreManager.h"
#include <SFML/Window/Joystick.hpp>
#include <stdexcept>
#include <cmath>
#include <iostream>

// --- FONCTION UTILITAIRE ---
static void adjustView(const sf::Vector2u& windowSize, sf::View& view, float targetRatio) {
    float windowRatio = static_cast<float>(windowSize.x) / static_cast<float>(windowSize.y);
    if (windowRatio > targetRatio) {
        // Écran plus large que le jeu (Bandes noires sur les côtés)
        float scaleFactor = targetRatio / windowRatio;
        view.setViewport(sf::FloatRect({(1.f - scaleFactor) / 2.f, 0.f}, {scaleFactor, 1.f}));
    } else {
        // Écran plus haut que le jeu (Bandes noires en haut/bas)
        float scaleFactor = windowRatio / targetRatio;
        view.setViewport(sf::FloatRect({0.f, (1.f - scaleFactor) / 2.f}, {1.f, scaleFactor}));
    }
}

// --- CLASSE ENGINE ---

Engine::Engine()
    : mCamera(sf::FloatRect({0.f, 0.f}, {Config::CAMERA_WIDTH, Config::CAMERA_HEIGHT})),
      mTimePerFrame(sf::seconds(Config::TIME_PER_FRAME)),
      mIsFullscreen(true), // Démarre directement en Fullscreen
      mHasFocus(true)
{
    mContextSettings.antiAliasingLevel = Config::ANTIALIASING_LEVEL;

    // Initialisation fenêtre
    recreateWindow();

    // --- LOG DE DEBUG ---
    // Affiche la résolution détectée pour confirmer le mode Desktop
    auto desktop = sf::VideoMode::getDesktopMode();
    std::cout << "[VIDEO] Desktop Resolution: " << desktop.size.x << "x" << desktop.size.y << std::endl;

    // --- Chargement Assets ---
    unsigned int maxTextureSize = sf::Texture::getMaximumSize();
    bool useSD = (maxTextureSize <= Config::TEXTURE_LIMIT_THRESHOLD);
    mAssetsManager.setUseSDAssets(useSD);
    std::string circuitFile = useSD ? Config::FILE_CIRCUIT_SD : Config::FILE_CIRCUIT_HD;

    if (!mAssetsManager.loadTexture("circuit", Config::TEXTURES_PATH + circuitFile) ||
        !mAssetsManager.loadTexture("voiture", Config::TEXTURES_PATH + "voiture.png")) {
        throw std::runtime_error("Failed to load textures");
    }

    mWorld = std::make_unique<World>(mWindow, mAssetsManager);

    if (!mAssetsManager.loadFont("arial", Config::FONTS_PATH + "arial.ttf")) {
        throw std::runtime_error("Failed to load font arial");
    }

    mMenu = std::make_unique<Menu>(mAssetsManager.getFont("arial"), mAssetsManager.getTexture("circuit"));
    mHud = std::make_unique<HUD>(mAssetsManager.getFont("arial"));
    mCameraManager = std::make_unique<Camera>(Config::CAMERA_WIDTH, Config::CAMERA_HEIGHT);
    mGameManager = std::make_unique<GameManager>();

    mCameraManager->update(mCamera, mWorld->getCar().getPosition(), mWorld->getTrackBounds().size);
    mHud->setBestTimes(mWorld->getGhost().getBestTimes());
}

void Engine::recreateWindow() {
    sf::VideoMode mode;
    std::uint32_t style;
    sf::State state;

    if (mIsFullscreen) {
        // --- VRAI FULLSCREEN NATIF ---
        // On récupère la résolution native de l'écran.
        mode = sf::VideoMode::getDesktopMode();

        // On demande explicitement l'état "Fullscreen" au système d'exploitation.
        // SFML s'occupera de retirer les bordures et de couvrir l'écran.
        style = sf::Style::Default;
        state = sf::State::Fullscreen;
    } else {
        // --- MODE FENÊTRÉ ---
        mode = sf::VideoMode({static_cast<unsigned int>(Config::WINDOW_WIDTH), static_cast<unsigned int>(Config::WINDOW_HEIGHT)});
        style = sf::Style::Default;
        state = sf::State::Windowed;
    }

    // Création de la fenêtre
    mWindow.create(mode, "RetroRush", style, state, mContextSettings);

    // --- POSITIONNEMENT ---
    // En mode Fullscreen natif, on ne doit JAMAIS utiliser setPosition.
    // Le système place la fenêtre automatiquement en (0,0).
    if (!mIsFullscreen) {
        auto desktop = sf::VideoMode::getDesktopMode();
        mWindow.setPosition({
            static_cast<int>(desktop.size.x - mode.size.x) / 2,
            static_cast<int>(desktop.size.y - mode.size.y) / 2
        });
    }

    // --- PARAMÈTRES VIDEO ---
    if (Config::ENABLE_VSYNC) {
        mWindow.setVerticalSyncEnabled(true);
        mWindow.setFramerateLimit(0);
    } else {
        mWindow.setVerticalSyncEnabled(false);
        if (Config::FRAME_LIMIT > 0) {
            mWindow.setFramerateLimit(Config::FRAME_LIMIT);
        }
    }

    // Cache le curseur en jeu si plein écran
    mWindow.setMouseCursorVisible(!mIsFullscreen);

    // Ajuste la caméra (Ratio aspect)
    adjustView(mWindow.getSize(), mCamera, Config::CAMERA_WIDTH / Config::CAMERA_HEIGHT);

    std::cout << "[VIDEO] Mode active: " << (mIsFullscreen ? "Fullscreen Natif" : "Fenetre")
              << " (" << mWindow.getSize().x << "x" << mWindow.getSize().y << ")" << std::endl;
}

void Engine::run() {
    sf::Clock clock;
    sf::Time timeSinceLastUpdate = sf::Time::Zero;
    sf::Clock fpsClock;
    int frameCount = 0;

    while (mWindow.isOpen()) {
        processEvents();

        // Pause CPU si perte de focus
        if (!mHasFocus) {
            sf::sleep(sf::milliseconds(100));
            clock.restart();
            continue;
        }

        sf::Time deltaTime = clock.restart();
        timeSinceLastUpdate += deltaTime;

        // Protection Lag (max 0.2s de rattrapage)
        if (timeSinceLastUpdate > sf::seconds(0.2f)) {
            timeSinceLastUpdate = sf::seconds(0.2f);
        }

        // Physique à pas fixe
        while (timeSinceLastUpdate > mTimePerFrame) {
            timeSinceLastUpdate -= mTimePerFrame;
            processEvents();
            if(mHasFocus) update(mTimePerFrame);
        }

        // Compteur FPS
        frameCount++;
        if (fpsClock.getElapsedTime().asSeconds() >= 1.f) {
            float currentFps = static_cast<float>(frameCount) / fpsClock.restart().asSeconds();
            mHud->updateFPS(currentFps, mWindow.getSize());
            frameCount = 0;
        }

        // Rendu interpolé
        float alpha = timeSinceLastUpdate.asSeconds() / mTimePerFrame.asSeconds();
        render(alpha);
    }
}

void Engine::toggleFullscreen() {
    mIsFullscreen = !mIsFullscreen;
    recreateWindow();
}

void Engine::processEvents() {
    while (auto eventOpt = mWindow.pollEvent()) {
        const sf::Event& event = *eventOpt;

        if (event.is<sf::Event::Closed>()) {
            mWindow.close();
        }
        else if (const auto* resizeEvent = event.getIf<sf::Event::Resized>()) {
            // Gestion du redimensionnement (ALT-TAB ou changement de résolution)
            adjustView(resizeEvent->size, mCamera, Config::CAMERA_WIDTH / Config::CAMERA_HEIGHT);
        }
        else if (event.is<sf::Event::FocusLost>()) {
            mHasFocus = false;
        }
        else if (event.is<sf::Event::FocusGained>()) {
            mHasFocus = true;
        }
        else if (const auto* keyEvent = event.getIf<sf::Event::KeyPressed>()) {
            if (keyEvent->code == sf::Keyboard::Key::Escape) mWindow.close();
            else if (keyEvent->code == sf::Keyboard::Key::F11) toggleFullscreen();
        }

        if (!mHasFocus) continue;
        if (mGameManager->isInMenu() || mGameManager->isFinished()) {
            bool startRequested = false;
             if (const auto* keyEvent = event.getIf<sf::Event::KeyPressed>()) {
                if (keyEvent->code == sf::Keyboard::Key::Enter || keyEvent->code == sf::Keyboard::Key::Space) startRequested = true;
            } else if (const auto* joyEvent = event.getIf<sf::Event::JoystickButtonPressed>()) {
                if (joyEvent->button == 0 || joyEvent->button == 7) startRequested = true;
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

    if (justStarted) mWorld->getPlayer().startClock();

    if (mGameManager->isPlaying() && !mGameManager->isTimerRunning()) {
        if (mWorld->isOnStartLine()) mGameManager->startTimer();
    }

    if (mGameManager->isPlaying()) {
        mWorld->update(deltaTime, mCamera);

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