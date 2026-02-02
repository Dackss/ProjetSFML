#include "CheckpointManager.h"

/// @brief Constructor
CheckpointManager::CheckpointManager() {}

/// @brief Assign collision mask
/// @param mask Reference to collision mask
void CheckpointManager::setCollisionMask(const CollisionMask* mask) {
    mCollisionMask = mask;
}

/// @brief Reset checkpoint counter and cooldown
void CheckpointManager::reset() {
    mCheckpointsPassed = 0;
    mCooldown.restart(); ///< Restart clock
}

/// @brief Check if checkpoint is crossed
/// @param position Current position
void CheckpointManager::update(const sf::Vector2f& position) {
    /// Ignore if less than 500ms since last checkpoint
    if (mCooldown.getElapsedTime().asMilliseconds() < 500) {
        return;
    }

    /// Check if position is on green checkpoint
    if (mCollisionMask->isOnGreen(position)) {
        mCheckpointsPassed++;
        mCooldown.restart(); ///< Reset cooldown
    }
}

/// @brief Check if all checkpoints are passed
/// @return True if lap complete
bool CheckpointManager::isLapComplete() const {
    return mCheckpointsPassed >= mCheckpointsToPass;
}

/// @brief Get number of validated checkpoints
/// @return Checkpoint count
int CheckpointManager::getCheckpointCount() const {
    return mCheckpointsPassed;
}

/// @brief Check if at least one checkpoint is passed
/// @return True if started
bool CheckpointManager::hasStarted() const {
    return mCheckpointsPassed > 0;
}

/// @brief Check if first checkpoint just crossed
/// @return True if first checkpoint recently crossed
bool CheckpointManager::justStartedLap() const {
    return mCheckpointsPassed == 1 && mCooldown.getElapsedTime().asMilliseconds() < 200;
}