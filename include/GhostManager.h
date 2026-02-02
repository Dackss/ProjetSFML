#ifndef GHOSTMANAGER_H
#define GHOSTMANAGER_H

#include <SFML/Graphics.hpp>
#include <vector>
#include "Config.h"
#include "Car.h"
#include "CheckpointManager.h"
#include "AssetsManager.h"

/// @brief Manages ghost car recording and playback
class GhostManager {
public:
    /// @brief Constructor
    /// @param assetsManager Resource manager
    explicit GhostManager(AssetsManager& assetsManager);

    /// @brief Update ghost state
    /// @param car Player car
    /// @param checkpoints Checkpoint manager
    void update(const Car& car, const CheckpointManager& checkpoints);

    /// @brief Render ghost
    /// @param window Render target
    /// @param isPlaying True if game is in playing state
    void render(sf::RenderWindow& window, bool isPlaying);

    /// @brief Reset ghost data
    void reset();

    /// @brief Handle lap completion
    /// @return True if lap processed
    bool handleLapComplete();

    /// @brief Submit race time
    /// @param time Race time
    void submitTime(sf::Time time);

    /// @brief Get best times
    /// @return Vector of best times
    const std::vector<sf::Time>& getBestTimes() const;

    /// @brief Set collision mask
    /// @param mask Collision mask reference
    void setCollisionMask(const CollisionMask* mask);

private:
    std::vector<sf::Vector2f> mGhostPositions;    ///< Ghost positions
    std::vector<float> mGhostRotations;           ///< Ghost rotations
    std::vector<sf::Vector2f> mNewGhostPositions; ///< Temporary positions
    std::vector<float> mNewGhostRotations;        ///< Temporary rotations
    bool mRecording;                              ///< Recording state
    bool mGhostEnabled;                           ///< Ghost enabled state
    bool mHasRecordingStarted;                    ///< Recording started flag
    bool mHasCrossedStart;                        ///< Start line crossed flag
    std::size_t mGhostIndex;                      ///< Current ghost frame
    int mGhostFrameCounter;                       ///< Frame counter
    sf::Sprite mGhostSprite;                      ///< Ghost sprite
    std::vector<sf::Time> mBestTimes;             ///< Best race times
    const CollisionMask* mCollisionMask;          ///< Collision mask
};

#endif // GHOSTMANAGER_H