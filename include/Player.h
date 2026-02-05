#ifndef PLAYER_H
#define PLAYER_H

#include "Car.h"
#include "CollisionMask.h"
#include <SFML/System/Clock.hpp>

/// @brief Manages player car and stats
class Player {
public:
    /// @brief Constructor
    /// @param carTexture Car texture
    explicit Player(sf::Texture& carTexture);

    /// @brief Update player state
    /// @param deltaTime Time since last update
    /// @param bounds Track bounds
    /// @param mask Collision mask
    void update(sf::Time deltaTime, const sf::FloatRect& bounds, const CollisionMask& mask);

    /// @brief Render player car
    /// @param window Render target
    void render(sf::RenderWindow& window, float alpha = 1.0f);

    /// @brief Reset player state
    void reset();

    /// @brief Start race timer
    void startClock();

    /// @brief Get player car
    /// @return Reference to car
    Car& getCar();

    const Car& getCar() const;

    /// @brief Get distance traveled
    /// @return Distance in meters
    float getDistance() const;

    /// @brief Get current lap
    /// @return Lap number
    int getLap() const;

    /// @brief Get elapsed race time
    /// @return Time since race start
    sf::Time getElapsedTime() const;

private:
    Car mCar;           ///< Player car
    float mDistance;    ///< Distance traveled
    int mLap;           ///< Current lap
    sf::Clock mClock;   ///< Race timer
};

#endif // PLAYER_H