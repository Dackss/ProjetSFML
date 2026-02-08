#include "World.h"
#include "Config.h"
#include <stdexcept>
#include <algorithm>
#include <cmath>

World::World(sf::RenderWindow& window, AssetsManager& assetsManager)
        : mWindow(window), mAssetsManager(assetsManager),
          mTrack(assetsManager.getTexture("circuit")),
          mPlayer(assetsManager.getTexture("voiture")),
          mGhost(assetsManager),
          mLapCount(0) {
    // ... (Chargement Mask inchangé) ...
    std::string maskFilename = mAssetsManager.isUsingSDAssets() ? Config::FILE_MASK_SD : Config::FILE_MASK_HD;
    if (!mCollisionMask.loadFromFile(Config::TEXTURES_PATH + maskFilename)) {
        throw std::runtime_error("Failed to load " + maskFilename);
    }

    const sf::Texture& circuitTexture = mAssetsManager.getTexture("circuit");
    sf::Vector2u texSize = circuitTexture.getSize();
    float scaleFactor = static_cast<float>(Config::WINDOW_WIDTH) / static_cast<float>(texSize.x);

    mTrack.setScale(scaleFactor);
    mCollisionMask.setScale(scaleFactor);
    mTrackSize = sf::Vector2f(texSize.x * scaleFactor, texSize.y * scaleFactor);

    mCheckpoints.setCollisionMask(&mCollisionMask);
}

void World::update(sf::Time deltaTime, sf::View& camera) {
    float dt = deltaTime.asSeconds();

    mPlayer.update(deltaTime, getTrackBounds(), mCollisionMask);
    mCheckpoints.update(mPlayer.getCar().getPosition());

    // Le ghost ne se mettra à jour que si startRace() a été appelé
    mGhost.update(dt, mPlayer.getCar());

    // --- Caméra Lag ---
    sf::Vector2f targetPos = mPlayer.getCar().getPosition();
    sf::Vector2f currentPos = camera.getCenter();
    sf::Vector2f viewSize = camera.getSize();

    float t = Config::CAMERA_LERP_SPEED * dt;
    sf::Vector2f newPos = currentPos + (targetPos - currentPos) * t;

    float minX = viewSize.x / 2.f;
    float maxX = std::max(minX, mTrackSize.x - viewSize.x / 2.f);
    float minY = viewSize.y / 2.f;
    float maxY = std::max(minY, mTrackSize.y - viewSize.y / 2.f);

    newPos.x = std::clamp(newPos.x, minX, maxX);
    newPos.y = std::clamp(newPos.y, minY, maxY);

    camera.setCenter(newPos);
}

void World::render(bool isPlaying, float alpha) {
    // ... (Culling et Track render inchangé) ...
    const sf::View& currentView = mWindow.getView();
    sf::Vector2f center = currentView.getCenter();
    sf::Vector2f size = currentView.getSize();
    float buffer = 100.f;
    sf::FloatRect viewBounds({center.x - size.x/2.f - buffer, center.y - size.y/2.f - buffer}, {size.x + buffer*2, size.y + buffer*2});
    sf::FloatRect trackRect({0.f, 0.f}, mTrackSize);

    if (viewBounds.findIntersection(trackRect)) {
        mTrack.render(mWindow);
    }

    mGhost.render(mWindow, isPlaying);
    mPlayer.render(mWindow, alpha);
}

// NOUVEAU
void World::startRace() {
    mGhost.startRecording();
}

sf::FloatRect World::getTrackBounds() const {
    return sf::FloatRect({0.f, 0.f}, mTrackSize);
}

bool World::isLapComplete() {
    bool touchedCheckpoint = mCheckpoints.isLapComplete();
    bool onBlue = mCollisionMask.isOnBlue(mPlayer.getCar().getPosition());

    if (touchedCheckpoint && onBlue) {
        if (mGhost.handleLapComplete()) {
            mCheckpoints.reset();
            mLapCount++;
            return true;
        }
    }
    return false;
}

void World::reset() {
    mCheckpoints.reset();
    mPlayer.reset();
    mGhost.reset();
    mLapCount = 0;
}

// Getters inchangés
Player& World::getPlayer() { return mPlayer; }
Car& World::getCar() { return mPlayer.getCar(); }
int World::getLapCount() const { return mLapCount; }
GhostManager& World::getGhost() { return mGhost; }
bool World::isOnStartLine() const { return mCollisionMask.isOnBlue(mPlayer.getCar().getPosition()); }