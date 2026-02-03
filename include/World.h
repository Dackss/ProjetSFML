#ifndef WORLD_H
#define WORLD_H

#include <SFML/Graphics.hpp>
#include "Track.h"
#include "Player.h"
#include "AssetsManager.h"
#include "CollisionMask.h"
#include "CheckpointManager.h"
#include "GhostManager.h"

/// @brief Manages game world
class World {
public:
    /// @brief Constructor
    /// @param window Render window
    /// @param assetsManager Resource manager
    explicit World(sf::RenderWindow& window, AssetsManager& assetsManager);

    /// @brief Update world state
    /// @param deltaTime Time since last update
    /// @param camera Camera view
    void update(sf::Time deltaTime, sf::View& camera);

    /// @brief Render world
    /// @param isPlaying True if game is in playing state
    void render(bool isPlaying, float alpha = 1.0f);

    /// @brief Get track bounds
    /// @return Track bounding rectangle
    sf::FloatRect getTrackBounds() const;

    /// @brief Get player
    /// @return Reference to player
    Player& getPlayer();

    /// @brief Get player car
    /// @return Reference to car
    Car& getCar();

    /// @brief Check if lap is complete
    /// @return True if lap complete
    bool isLapComplete();

    /// @brief Reset world state
    void reset();

    /// @brief Get lap count
    /// @return Number of laps
    int getLapCount() const;

    /// @brief Get ghost manager
    /// @return Reference to ghost manager
    GhostManager& getGhost();

private:
    sf::RenderWindow& mWindow;    ///< Render window
    AssetsManager& mAssetsManager;///< Resource manager
    Track mTrack;                 ///< Track object
    Player mPlayer;               ///< Player object
    CollisionMask mCollisionMask; ///< Collision mask
    CheckpointManager mCheckpoints;///< Checkpoint manager
    GhostManager mGhost;          ///< Ghost manager
    sf::Vector2f mTrackSize;      ///< Scaled track size
    float mTrackScale;            ///< Track scale factor
    int mLapCount;                ///< Lap counter
};

#endif // WORLD_H