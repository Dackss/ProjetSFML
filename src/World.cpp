#include "World.h"
#include "Config.h"
#include <stdexcept>

/// @brief Constructor
/// @param window Render window
/// @param assetsManager Resource manager
#include "World.h"
#include "Config.h"
#include <stdexcept>

World::World(sf::RenderWindow& window, AssetsManager& assetsManager)
        : mWindow(window), mAssetsManager(assetsManager),
          mTrack(assetsManager.getTexture("circuit")),
          mPlayer(assetsManager.getTexture("voiture")),
          mGhost(assetsManager),
          mLapCount(0) {

    // Chargement du masque
    if (!mCollisionMask.loadFromFile(Config::TEXTURES_PATH + "circuit_mask.png")) {
        throw std::runtime_error("Failed to load circuit_mask.png");
    }

    // --- CORRECTION MAJEURE ICI ---
    // On calcule l'échelle pour que le circuit ait TOUJOURS la taille prévue dans Config.h
    // Peu importe si l'image est 4K ou HD, elle sera redimensionnée pour faire 'WINDOW_WIDTH' pixels de large.
    sf::Vector2u texSize = mAssetsManager.getTexture("circuit").getSize();

    // On utilise Config::WINDOW_WIDTH pour garantir que la voiture (placée selon Config) soit sur la route
    float scaleFactor = static_cast<float>(Config::WINDOW_WIDTH) / static_cast<float>(texSize.x);

    // On applique cette échelle partout
    mTrack.setScale(scaleFactor);
    mCollisionMask.setScale(scaleFactor);

    // On stocke la taille réelle du monde pour la caméra
    mTrackSize = sf::Vector2f(texSize.x * scaleFactor, texSize.y * scaleFactor);

    mCheckpoints.setCollisionMask(&mCollisionMask);
    mGhost.setCollisionMask(&mCollisionMask);
}

/// @brief Update world state
/// @param deltaTime Time since last update
/// @param camera Camera view
/// @see https://www.sfml-dev.org/documentation/3.0.0/classsf_1_1View.php#a24d0503c14555f9d4f5374ad80968023
void World::update(sf::Time deltaTime, sf::View& camera) {
    mPlayer.update(deltaTime, getTrackBounds(), mCollisionMask);
    mCheckpoints.update(mPlayer.getCar().getPosition());
    mGhost.update(mPlayer.getCar(), mCheckpoints);

    // Caméra centrée sur la voiture, bornée au circuit
    sf::Vector2f carPos = mPlayer.getCar().getPosition();
    sf::Vector2f viewSize = camera.getSize();
    float minX = viewSize.x / 2.f;
    float maxX = mTrackSize.x - viewSize.x / 2.f;
    float minY = viewSize.y / 2.f;
    float maxY = mTrackSize.y - viewSize.y / 2.f;

    // Sécurité si le circuit est plus petit que la caméra (rare mais possible)
    if (maxX < minX) maxX = minX;
    if (maxY < minY) maxY = minY;

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