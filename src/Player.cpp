#include "Player.h"
#include "Config.h"

/// @brief Constructor
/// @param carTexture Car texture
Player::Player(sf::Texture& carTexture)
        : mCar(carTexture), mDistance(0.f), mLap(0) {}

/// @brief Update player state
/// @param deltaTime Time since last update
/// @param bounds Track bounds
/// @param mask Collision mask
void Player::update(sf::Time deltaTime, const sf::FloatRect& bounds, const CollisionMask& mask) {
    /// Update car movement
    mCar.update(deltaTime, bounds, mask);

    /// Update distance traveled
    mDistance += mCar.getSpeed() * deltaTime.asSeconds();
}

/// @brief Render player car
/// @param window Render target
void Player::render(sf::RenderWindow& window) {
    mCar.render(window);
}

/// @brief Reset player state
void Player::reset() {
    /// Reset car position and rotation
    mCar.setPosition({Config::CAR_INITIAL_POS_X, Config::CAR_INITIAL_POS_Y});
    mCar.setRotation(Config::CAR_INITIAL_ROTATION);
    mCar.resetVelocity();

    /// Reset stats
    mDistance = 0.f;
    mLap = 0;
}

/// @brief Start race timer
void Player::startClock() {
    mClock.restart();
}

/// @brief Get player car
/// @return Reference to car
Car& Player::getCar() {
    return mCar;
}

/// @brief Get distance traveled
/// @return Distance in meters
float Player::getDistance() const {
    return mDistance;
}

/// @brief Get current lap
/// @return Lap number
int Player::getLap() const {
    return mLap;
}

/// @brief Get elapsed race time
/// @return Time since race start
sf::Time Player::getElapsedTime() const {
    return mClock.getElapsedTime();
}