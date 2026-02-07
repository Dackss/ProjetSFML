#include "Engine.h"
#include "Config.h"
#include "ScoreManager.h"
#include <SFML/Window/Joystick.hpp>
#include <stdexcept>

// --- FONCTION UTILITAIRE ---
static void adjustView(const sf::Vector2u& windowSize, sf::View& view, float targetRatio) {
    float windowRatio = static_cast<float>(windowSize.x) / static_cast<float>(windowSize.y);
    if (windowRatio > targetRatio) {
        float scaleFactor = targetRatio / windowRatio;
        view.setViewport(sf::FloatRect({(1.f - scaleFactor) / 2.f, 0.f}, {scaleFactor, 1.f}));
    } else {
        float scaleFactor = windowRatio / targetRatio;
        view.setViewport(sf::FloatRect({0.f, (1.f - scaleFactor) / 2.f}, {1.f, scaleFactor}));
    }
}

Engine::Engine()
    : mCamera(sf::FloatRect({0.f, 0.f}, {Config::CAMERA_WIDTH, Config::CAMERA_HEIGHT})),
      mTimePerFrame(sf::seconds(Config::TIME_PER_FRAME)),
      mIsFullscreen(true),
      mHasFocus(true)
{
    mContextSettings.antiAliasingLevel = Config::ANTIALIASING_LEVEL;

    recreateWindow();

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
    sf::VideoMode mode = mIsFullscreen ? sf::VideoMode::getDesktopMode()
                                       : sf::VideoMode({static_cast<unsigned int>(Config::WINDOW_WIDTH), static_cast<unsigned int>(Config::WINDOW_HEIGHT)});

    auto style = mIsFullscreen ? sf::Style::None : sf::Style::Default;
    auto state = mIsFullscreen ? sf::State::Fullscreen : sf::State::Windowed;

    mWindow.create(mode, "RetroRush", style, state, mContextSettings);

    if (!mIsFullscreen) {
        auto desktop = sf::VideoMode::getDesktopMode();
        mWindow.setPosition({
            static_cast<int>(desktop.size.x - mode.size.x) / 2,
            static_cast<int>(desktop.size.y - mode.size.y) / 2
        });
    }

    mWindow.setVerticalSyncEnabled(Config::ENABLE_VSYNC);
    mWindow.setFramerateLimit(Config::FRAME_LIMIT);

    mWindow.setMouseCursorVisible(!mIsFullscreen);
    adjustView(mWindow.getSize(), mCamera, Config::CAMERA_WIDTH / Config::CAMERA_HEIGHT);
}

void Engine::run() {
    sf::Clock clock;
    sf::Time timeSinceLastUpdate = sf::Time::Zero;
    sf::Clock fpsClock;
    int frameCount = 0;

    while (mWindow.isOpen()) {
        processEvents();

        if (!mHasFocus) {
            sf::sleep(sf::milliseconds(100));
            clock.restart();
            continue;
        }

        sf::Time deltaTime = clock.restart();
        timeSinceLastUpdate += deltaTime;

        if (timeSinceLastUpdate > sf::seconds(0.2f)) {
            timeSinceLastUpdate = sf::seconds(0.2f);
        }

        while (timeSinceLastUpdate > mTimePerFrame) {
            timeSinceLastUpdate -= mTimePerFrame;
            update(mTimePerFrame);
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
    recreateWindow();
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
            if (keyEvent->code == sf::Keyboard::Key::Escape) mWindow.close();
            else if (keyEvent->code == sf::Keyboard::Key::F11) toggleFullscreen();
        }

        if (!mHasFocus) continue;

        if (mGameManager->isInMenu() || mGameManager->isFinished()) {
            bool startRequested = false;
            if (const auto* keyEvent = event.getIf<sf::Event::KeyPressed>()) {
                if (keyEvent->code == sf::Keyboard::Key::Enter || keyEvent->code == sf::Keyboard::Key::Space)
                    startRequested = true;
            } else if (const auto* joyEvent = event.getIf<sf::Event::JoystickButtonPressed>()) {
                if (joyEvent->button == 0 || joyEvent->button == 7)
                    startRequested = true;
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
            mGameManager->markLapFinished(mGameManager->getRaceTime());
            mMenu->setResultText(mGameManager->getResultText());
            mWorld->getGhost().submitTime(mWorld->getPlayer().getElapsedTime());
            mHud->setBestTimes(mWorld->getGhost().getBestTimes());
        }
    }

    float speed = mWorld->getCar().getSpeed() * 3.6f;
    int countdown = mGameManager->isCountdown() ? mGameManager->getCountdownValue() : -2;
    mHud->update(speed, mGameManager->getRaceTime(), countdown, mWindow.getSize());
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