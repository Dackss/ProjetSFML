#include "World.h"
#include "Config.h"
#include <stdexcept>
#include <algorithm> // Pour std::clamp, std::max, std::min
#include <cmath>     // Pour std::abs si nécessaire

World::World(sf::RenderWindow& window, AssetsManager& assetsManager)
        : mWindow(window), mAssetsManager(assetsManager),
          mTrack(assetsManager.getTexture("circuit")),
          mPlayer(assetsManager.getTexture("voiture")),
          mGhost(assetsManager),
          mLapCount(0) {

    std::string maskFilename;
    if (mAssetsManager.isUsingSDAssets()) {
        maskFilename = Config::FILE_MASK_SD;
    } else {
        maskFilename = Config::FILE_MASK_HD;
    }
    if (!mCollisionMask.loadFromFile(Config::TEXTURES_PATH + maskFilename)) {
        throw std::runtime_error("Failed to load " + maskFilename);
    }
    const sf::Texture& circuitTexture = mAssetsManager.getTexture("circuit");
    sf::Vector2u texSize = circuitTexture.getSize();
    float scaleFactor = static_cast<float>(Config::WINDOW_WIDTH) / static_cast<float>(texSize.x);

    // Vérification de sécurité pour Raspberry Pi (Texture max size)
    unsigned int maxSize = sf::Texture::getMaximumSize();
    if (texSize.x > maxSize || texSize.y > maxSize) {
        fprintf(stderr, "WARNING: Texture size (%u, %u) exceeds hardware limit (%u)!\n",
                texSize.x, texSize.y, maxSize);
    }

    mTrack.setScale(scaleFactor);
    mCollisionMask.setScale(scaleFactor);

    // Taille réelle du monde
    mTrackSize = sf::Vector2f(texSize.x * scaleFactor, texSize.y * scaleFactor);

    // Injection des dépendances
    mCheckpoints.setCollisionMask(&mCollisionMask);
    mGhost.setCollisionMask(&mCollisionMask);
}

void World::update(sf::Time deltaTime, sf::View& camera) {
    mPlayer.update(deltaTime, getTrackBounds(), mCollisionMask);
    mCheckpoints.update(mPlayer.getCar().getPosition());
    mGhost.update(mPlayer.getCar(), mCheckpoints);

    // --- Gestion Caméra ---
    sf::Vector2f carPos = mPlayer.getCar().getPosition();
    sf::Vector2f viewSize = camera.getSize();

    // Clamp de la caméra pour ne pas sortir du circuit
    float minX = viewSize.x / 2.f;
    float maxX = std::max(minX, mTrackSize.x - viewSize.x / 2.f);

    float minY = viewSize.y / 2.f;
    float maxY = std::max(minY, mTrackSize.y - viewSize.y / 2.f);

    carPos.x = std::clamp(carPos.x, minX, maxX);
    carPos.y = std::clamp(carPos.y, minY, maxY);

    camera.setCenter(carPos);
}

void World::render(bool isPlaying, float alpha) {
    // 1. Définition de la vue et du Culling
    const sf::View& currentView = mWindow.getView();
    sf::Vector2f center = currentView.getCenter();
    sf::Vector2f size = currentView.getSize();

    // Marge de sécurité pour éviter le clipping sur les bords
    float buffer = 100.f;

    // CORRECTION SFML 3 : On utilise des accolades {} pour créer des Vector2f
    sf::FloatRect viewBounds(
        {center.x - size.x / 2.f - buffer, center.y - size.y / 2.f - buffer}, // Position (Vector2f)
        {size.x + buffer * 2.f, size.y + buffer * 2.f}                        // Taille (Vector2f)
    );

    // 2. Rendu du Circuit
    // SFML 3 : Rect constructeur prend (Position, Taille)
    sf::FloatRect trackRect({0.f, 0.f}, mTrackSize);

    if (viewBounds.findIntersection(trackRect)) {
        mTrack.render(mWindow);
    }

    // 3. Rendu du Ghost
    mGhost.render(mWindow, isPlaying, alpha);

    // 4. Rendu du Joueur
    mPlayer.render(mWindow, alpha);
}

sf::FloatRect World::getTrackBounds() const {
    // Correction ici aussi par sécurité pour SFML 3
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

Player& World::getPlayer() { return mPlayer; }
Car& World::getCar() { return mPlayer.getCar(); }
int World::getLapCount() const { return mLapCount; }
GhostManager& World::getGhost() { return mGhost; }

bool World::isOnStartLine() const {
    return mCollisionMask.isOnBlue(mPlayer.getCar().getPosition());
}