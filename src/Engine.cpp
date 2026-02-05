#include "Engine.h"
#include "ScoreManager.h"
#include <SFML/Window/Joystick.hpp>
#include <stdexcept>
#include <cmath>

/// @brief Helper to maintain aspect ratio
static void adjustView(const sf::Vector2u& windowSize, sf::View& view, float targetRatio) {
    float windowRatio = static_cast<float>(windowSize.x) / static_cast<float>(windowSize.y);

    if (windowRatio > targetRatio) {
        // Fenêtre plus large que le jeu
        float scaleFactor = targetRatio / windowRatio;
        view.setViewport(sf::FloatRect({(1.f - scaleFactor) / 2.f, 0.f}, {scaleFactor, 1.f}));
    } else {
        // Fenêtre plus haute que le jeu
        float scaleFactor = windowRatio / targetRatio;
        view.setViewport(sf::FloatRect({0.f, (1.f - scaleFactor) / 2.f}, {1.f, scaleFactor}));
    }
}

Engine::Engine()
    // CORRECTION ICI : L'ordre doit suivre celui de Engine.h (Resources -> State)
    : mCamera(sf::FloatRect({0.f, 0.f}, {Config::CAMERA_WIDTH, Config::CAMERA_HEIGHT})),
      mTimePerFrame(sf::seconds(Config::TIME_PER_FRAME)),
      mIsFullscreen(true),
      mHasFocus(true)
{
    // Configuration de l'anti-aliasing (SFML 3 : antiAliasingLevel avec un grand 'A')
    mContextSettings.antiAliasingLevel = 0;

    // Création de la fenêtre (SFML 3 : create prend VideoMode, Titre, Style, State, Settings)
    // Pour le fullscreen, le style est ignoré ou mis à Default, et l'état est State::Fullscreen
    mWindow.create(
        sf::VideoMode::getDesktopMode(),
        "RetroRush",
        sf::Style::Default,
        sf::State::Fullscreen,
        mContextSettings
    );

    mWindow.setVerticalSyncEnabled(true);
    mWindow.setMouseCursorVisible(false);

    // Ajustement immédiat du ratio
    adjustView(mWindow.getSize(), mCamera, Config::CAMERA_WIDTH / Config::CAMERA_HEIGHT);

    unsigned int maxTextureSize = sf::Texture::getMaximumSize();
    printf("GPU Max Texture Size: %u px\n", maxTextureSize);
    std::string circuitFile;

    if (maxTextureSize <= 4096 || maxTextureSize == 16384) {
        printf("FORCING SD MODE: Low spec or Driver issue detected.\n");
        mAssetsManager.setUseSDAssets(true);
        circuitFile = Config::FILE_CIRCUIT_SD;
    } else {
        mAssetsManager.setUseSDAssets(false);
        circuitFile = Config::FILE_CIRCUIT_HD;
    }

    /// Load textures
    std::string circuitPath = Config::TEXTURES_PATH + circuitFile;
    std::string voiturePath = Config::TEXTURES_PATH + "voiture.png";

    if (!mAssetsManager.loadTexture("circuit", circuitPath) ||
        !mAssetsManager.loadTexture("voiture", voiturePath)) {
        throw std::runtime_error("Failed to load textures: " + circuitPath); // Debug facilité
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
            if (mHasFocus) {
                update(mTimePerFrame);
            }
        }
        float alpha = timeSinceLastUpdate.asSeconds() / mTimePerFrame.asSeconds();

        render(alpha);
    }
}

void Engine::toggleFullscreen() {
    mIsFullscreen = !mIsFullscreen;

    sf::VideoMode mode;
    sf::State state;
    std::uint32_t style;

    if (mIsFullscreen) {
        mode = sf::VideoMode::getDesktopMode();
        state = sf::State::Fullscreen;
        style = sf::Style::Default;
    } else {
        // SFML 3 : VideoMode prend un Vector2u ({w, h})
        mode = sf::VideoMode({static_cast<unsigned int>(Config::WINDOW_WIDTH), static_cast<unsigned int>(Config::WINDOW_HEIGHT)});
        state = sf::State::Windowed;
        style = sf::Style::Default;
    }

    // Re-création avec la signature SFML 3
    mWindow.create(mode, "RetroRush", style, state, mContextSettings);

    mWindow.setVerticalSyncEnabled(true);
    mWindow.setMouseCursorVisible(!mIsFullscreen);

    if (!mIsFullscreen) {
        auto desktop = sf::VideoMode::getDesktopMode();
        // SFML 3 : VideoMode.size.x au lieu de .width
        sf::Vector2i position(
            (desktop.size.x - mode.size.x) / 2,
            (desktop.size.y - mode.size.y) / 2
        );
        mWindow.setPosition(position);
    }

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
        // SFML 3 : FocusLost / FocusGained (inversion des termes par rapport à v2)
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

        if (!mHasFocus) return;

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