#include "GhostManager.h"
#include <algorithm>
#include <fstream>

static sf::Vector2f lerp(const sf::Vector2f& a, const sf::Vector2f& b, float t) {
    return a + (b - a) * t;
}

static float lerpAngle(float a, float b, float t) {
    float diff = b - a;
    // Gestion du bouclage 0° <-> 360°
    while (diff > 180.f) diff -= 360.f;
    while (diff < -180.f) diff += 360.f;
    return a + diff * t;
}

/// @brief Constructor
/// @param assetsManager Resource manager
GhostManager::GhostManager(AssetsManager& assetsManager)
        : mGhostSprite(assetsManager.getTexture("voiture")) {
    mGhostSprite.setOrigin(mGhostSprite.getLocalBounds().size / 2.f);

    // Charger le fantôme s'il existe déjà sur le disque
    loadGhostFromFile("best_ghost.dat");
}

/// @brief Reset ghost data
void GhostManager::reset() {
    mGhostIndex = 0;
    mGhostFrameCounter = 0;
    mRecording = false;
    mHasRecordingStarted = false;
    mHasCrossedStart = false;
    mNewGhostPositions.clear();
    mNewGhostRotations.clear();
    mNewGhostPositions.reserve(12000);
    mNewGhostRotations.reserve(12000);
}

/// @brief Update ghost state
/// @param car Player car
/// @param checkpoints Checkpoint manager
void GhostManager::update(const Car& car, const CheckpointManager& checkpoints) {
    (void)checkpoints; // Indique explicitement que le paramètre est ignoré pour l'instant

    if (mRecording) {
        mNewGhostPositions.push_back(car.getPosition());
        mNewGhostRotations.push_back(car.getRotation());
    }

    /// Start recording when crossing start line
    if (!mRecording && !mHasRecordingStarted && mCollisionMask->isOnBlue(car.getPosition())) {
        mRecording = true;
        mHasRecordingStarted = true;
    }

    /// Enable ghost display after crossing start line
    if (!mHasCrossedStart && mCollisionMask->isOnBlue(car.getPosition())) {
        mHasCrossedStart = true;
    }

    if (mGhostEnabled && mHasCrossedStart && mGhostIndex + 1 < mGhostPositions.size()) {
        mGhostIndex++;
    }
}

/// @brief Render ghost
/// @param window Render target
/// @param isPlaying True if game is in playing state
void GhostManager::render(sf::RenderWindow& window, bool isPlaying, float alpha) {
    // Vérification de sécurité : on doit avoir au moins 2 points pour interpoler
    if (mGhostEnabled && mHasCrossedStart && isPlaying && mGhostIndex < mGhostPositions.size()) {

        sf::Vector2f currentPos = mGhostPositions[mGhostIndex];
        float currentRot = mGhostRotations[mGhostIndex];

        size_t nextIndex = mGhostIndex + 1;
        if (nextIndex >= mGhostPositions.size()) {
            nextIndex = mGhostIndex;
        }

        sf::Vector2f nextPos = mGhostPositions[nextIndex];
        float nextRot = mGhostRotations[nextIndex];

        // Calcul de la position interpolée fluide
        sf::Vector2f drawPos = lerp(currentPos, nextPos, alpha);
        float drawRot = lerpAngle(currentRot, nextRot, alpha);

        /// Update ghost sprite properties
        mGhostSprite.setPosition(drawPos);
        mGhostSprite.setRotation(sf::degrees(drawRot));
        mGhostSprite.setColor(sf::Color(0, 200, 255, 120));
        mGhostSprite.setScale({Config::CAR_SCALE, Config::CAR_SCALE});

        window.draw(mGhostSprite);

        // Note : L'incrémentation de mGhostIndex reste dans update() !
    }
}

/// @brief Handle lap completion
/// @return True if lap processed
bool GhostManager::handleLapComplete() {
    if (mRecording) {
        if (mNewGhostPositions.size() < 50) return false;

        sf::Time newTime = sf::seconds(static_cast<float>(mNewGhostPositions.size()) / Config::FPS);
        sf::Time oldTime = sf::seconds(static_cast<float>(mGhostPositions.size()) / Config::FPS);

        if (!mGhostEnabled || newTime < oldTime) {
            mGhostPositions = mNewGhostPositions;
            mGhostRotations = mNewGhostRotations;
            mGhostEnabled = true;

            saveGhostToFile("best_ghost.dat");
        }

        /// Clear temporary data
        mNewGhostPositions.clear();
        mNewGhostRotations.clear();
        mRecording = false;
        mGhostIndex = 0;
        mGhostFrameCounter = 0;
    }

    return true;
}

/// @brief Submit race time
/// @param time Race time
void GhostManager::submitTime(sf::Time time) {
    mBestTimes.push_back(time);
    std::sort(mBestTimes.begin(), mBestTimes.end());

    /// Keep top 3 times
    if (mBestTimes.size() > 3) {
        mBestTimes.pop_back();
    }
}

/// @brief Get best times
/// @return Vector of best times
const std::vector<sf::Time>& GhostManager::getBestTimes() const {
    return mBestTimes;
}

/// @brief Set collision mask
/// @param mask Collision mask reference
void GhostManager::setCollisionMask(const CollisionMask* mask) {
    mCollisionMask = mask;
}

void GhostManager::saveGhostToFile(const std::string& filename) {
    std::ofstream file(filename, std::ios::binary);
    if (!file) return;

    size_t size = mGhostPositions.size();
    // 1. Sauvegarde la taille du vecteur
    file.write(reinterpret_cast<const char*>(&size), sizeof(size));

    if (size > 0) {
        // 2. Sauvegarde les positions
        file.write(reinterpret_cast<const char*>(mGhostPositions.data()), size * sizeof(sf::Vector2f));
        // 3. Sauvegarde les rotations
        file.write(reinterpret_cast<const char*>(mGhostRotations.data()), size * sizeof(float));
    }
    file.close();
}

void GhostManager::loadGhostFromFile(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file) return;

    size_t size;
    // 1. Lit la taille
    file.read(reinterpret_cast<char*>(&size), sizeof(size));

    if (size > 0) {
        mGhostPositions.resize(size);
        mGhostRotations.resize(size);
        // 2. Lit les positions
        file.read(reinterpret_cast<char*>(mGhostPositions.data()), size * sizeof(sf::Vector2f));
        // 3. Lit les rotations
        file.read(reinterpret_cast<char*>(mGhostRotations.data()), size * sizeof(float));

        mGhostEnabled = true; // Active le fantôme s'il y a des données
    }
    file.close();
}