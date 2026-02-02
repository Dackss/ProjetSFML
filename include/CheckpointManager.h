#ifndef CHECKPOINTMANAGER_H
#define CHECKPOINTMANAGER_H

#include <SFML/Graphics.hpp>
#include "CollisionMask.h"

/// @brief Manages checkpoints for a lap
class CheckpointManager {
public:
    /// @brief Constructor
    CheckpointManager();

    /// @brief Assign collision mask
    /// @param mask Reference to collision mask
    void setCollisionMask(const CollisionMask* mask);

    /// @brief Reset checkpoint counter
    void reset();

    /// @brief Check if checkpoint is crossed
    /// @param position Current position
    void update(const sf::Vector2f& position);

    /// @brief Check if all checkpoints are passed
    /// @return True if lap complete
    bool isLapComplete() const;

    /// @brief Get number of validated checkpoints
    /// @return Checkpoint count
    int getCheckpointCount() const;

    /// @brief Check if at least one checkpoint is passed
    /// @return True if started
    bool hasStarted() const;

    /// @brief Check if first checkpoint just crossed
    /// @return True if first checkpoint recently crossed
    bool justStartedLap() const;

private:
    const CollisionMask* mCollisionMask = nullptr; ///< Collision mask reference
    int mCheckpointsPassed = 0;             ///< Number of checkpoints passed
    int mCheckpointsToPass = 1;             ///< Total checkpoints to pass
    sf::Clock mCooldown;                ///< Cooldown to prevent double counting
};

#endif // CHECKPOINTMANAGER_H