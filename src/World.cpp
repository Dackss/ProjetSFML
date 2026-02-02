#include "World.h"
#include "Config.h"
#include <stdexcept>

/// @brief Constructor
/// @param window Render window
/// @param assetsManager Resource manager
World::World(sf::RenderWindow& window, AssetsManager& assetsManager)
        : mWindow(window), mAssetsManager(assetsManager), mTrack(assetsManager.getTexture("circuit"), window.getSize()),
          mPlayer(assetsManager.getTexture("voiture")), mGhost(assetsManager), mLapCount(0) {
    /// Load collision mask
    if (!mCollisionMask.loadFromFile(Config::TEXTURES_PATH + "circuit_mask.png")) {
        throw std::runtime_error("Failed to load circuit_mask.png");
    }

    /// Calculate track scale
    sf::Vector2u textureSize = mAssetsManager.getTexture("circuit").getSize();
    float windowWidth = window.getSize().x;
    float windowHeight = window.getSize().y;
    mTrackScale = std::max(windowWidth / textureSize.x, windowHeight / textureSize.y);
    mTrackSize = sf::Vector2f(textureSize.x * mTrackScale, textureSize.y * mTrackScale);

    /// Apply scale to collision mask
    mCollisionMask.setScale(mTrackScale);

    /// Set collision mask for checkpoints and ghost
    mCheckpoints.setCollisionMask(&mCollisionMask);
    mGhost.setCollisionMask(&mCollisionMask);
}

/// @brief Update world state
/// @param deltaTime Time since last update
/// @param camera Camera view
/// @see https://www.sfml-dev.org/documentation/3.0.0/classsf_1_1View.php#a24d0503c14555f9d4f5374ad80968023
void World::update(sf::Time deltaTime, sf::View& camera) {
    /// Update game objects
    mPlayer.update(deltaTime, getTrackBounds(), mCollisionMask);
    mCheckpoints.update(mPlayer.getCar().getPosition());
    mGhost.update(mPlayer.getCar(), mCheckpoints);

    /// Center camera on car
    sf::Vector2f carPos = mPlayer.getCar().getPosition();
    sf::Vector2f viewSize = camera.getSize();
    float minX = viewSize.x / 2.f;
    float maxX = mTrackSize.x - viewSize.x / 2.f;
    float minY = viewSize.y / 2.f;
    float maxY = mTrackSize.y - viewSize.y / 2.f;
    carPos.x = std::max(minX, std::min(carPos.x, maxX));
    carPos.y = std::max(minY, std::min(carPos.y, maxY));
    camera.setCenter(carPos);
}

/// @brief Render world
/// @param isPlaying True if game is in playing state
/// @see https://www.sfml-dev.org/documentation/3.0.0/classsf_1_1RenderWindow.php#a839bbf336bd120d2a91d87a47f5296b2
void World::render(bool isPlaying) {
    mTrack.render(mWindow);
    mGhost.render(mWindow, isPlaying);
    mPlayer.render(mWindow);
}

/// @brief Get track bounds
/// @return Track bounding rectangle
sf::FloatRect World::getTrackBounds() const {
    return sf::FloatRect({0.f, 0.f}, mTrackSize);
}

/// @brief Check if lap is complete
/// @return True if lap complete
bool World::isLapComplete() {
    bool touchedCheckpoint = mCheckpoints.isLapComplete();
    bool onBlue = mCollisionMask.isOnBlue(mPlayer.getCar().getPosition());
    if (!(touchedCheckpoint && onBlue)) {
        return false;
    }
    if (!mGhost.handleLapComplete()) {
        return false;
    }
    mCheckpoints.reset();
    mLapCount++;
    return true;
}

/// @brief Reset world state
void World::reset() {
    mCheckpoints.reset();
    mPlayer.reset();
    mGhost.reset();
    mLapCount = 0;
}

/// @brief Get player
/// @return Reference to player
Player& World::getPlayer() {
    return mPlayer;
}

/// @brief Get player car
/// @return Reference to car
Car& World::getCar() {
    return mPlayer.getCar();
}

/// @brief Get lap count
/// @return Number of laps
int World::getLapCount() const {
    return mLapCount;
}

/// @brief Get ghost manager
/// @return Reference to ghost manager
GhostManager& World::getGhost() {
    return mGhost;
}