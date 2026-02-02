#include "GhostManager.h"
#include <algorithm>

/// @brief Constructor
/// @param assetsManager Resource manager
GhostManager::GhostManager(AssetsManager& assetsManager)
        : mGhostSprite(assetsManager.getTexture("voiture")) {
    /// Set sprite origin to center
    mGhostSprite.setOrigin(mGhostSprite.getLocalBounds().size / 2.f);
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
}

/// @brief Render ghost
/// @param window Render target
/// @param isPlaying True if game is in playing state
void GhostManager::render(sf::RenderWindow& window, bool isPlaying) {
    if (mGhostEnabled && mHasCrossedStart && isPlaying && mGhostIndex < mGhostPositions.size()) {
        /// Update ghost sprite properties
        mGhostSprite.setPosition(mGhostPositions[mGhostIndex]);
        mGhostSprite.setRotation(sf::degrees(mGhostRotations[mGhostIndex]));
        mGhostSprite.setColor(sf::Color(0, 200, 255, 120)); ///< Blue transparent color
        mGhostSprite.setScale({Config::CAR_SCALE, Config::CAR_SCALE});
        window.draw(mGhostSprite);

        mGhostFrameCounter++;
        if (mGhostIndex + 1 < mGhostPositions.size()) {
            mGhostIndex++;
        }
    }
}

/// @brief Handle lap completion
/// @return True if lap processed
bool GhostManager::handleLapComplete() {
    if (mRecording) {
        /// Ignore short runs
        if (mNewGhostPositions.size() < 50) {
            return false;
        }

        /// Compare new and old ghost times
        sf::Time newTime = sf::seconds(static_cast<float>(mNewGhostPositions.size()) / Config::FPS);
        sf::Time oldTime = sf::seconds(static_cast<float>(mGhostPositions.size()) / Config::FPS);

        if (!mGhostEnabled || newTime < oldTime) {
            mGhostPositions = mNewGhostPositions;
            mGhostRotations = mNewGhostRotations;
            mGhostEnabled = true;
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