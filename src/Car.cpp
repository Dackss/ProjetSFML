#include "Car.h"
#include "Config.h"
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Joystick.hpp> // Ajout pour le support manette
#include <cmath>

/// @brief Constructor with texture
/// @param texture Sprite texture
Car::Car(sf::Texture& texture) : mSprite(texture), mVelocity(0.f, 0.f) {
    mSprite.setPosition({Config::CAR_INITIAL_POS_X, Config::CAR_INITIAL_POS_Y});
    mSprite.setRotation(sf::degrees(Config::CAR_INITIAL_ROTATION));
    mSprite.setScale({Config::CAR_SCALE, Config::CAR_SCALE});

    /// Set origin to center for natural rotation
    sf::Vector2u textureSize = texture.getSize();
    mSprite.setOrigin({textureSize.x / 2.f, textureSize.y / 2.f});
}

/// @brief Update car physics and position
/// @param deltaTime Time since last update
/// @param trackBounds Circuit boundaries
/// @param mask Collision mask
void Car::update(sf::Time deltaTime, const sf::FloatRect& trackBounds, const CollisionMask& mask) {
    float dt = deltaTime.asSeconds(); ///< Time delta in seconds

    /// Calculate speed magnitude
    float speed = std::sqrt(mVelocity.x * mVelocity.x + mVelocity.y * mVelocity.y);

    /// Reduce rotation at low speed
    float turnFactor = std::min(speed / 20.f, 1.f);

    // --- GESTION DES ENTRÉES (CLAVIER + MANETTE) ---

    // Joystick 0 par défaut
    unsigned int joystickId = 0;
    float joyX = sf::Joystick::getAxisPosition(joystickId, sf::Joystick::Axis::X);
    bool joyLeft = joyX < -40.f; // Deadzone de 40%
    bool joyRight = joyX > 40.f;

    /// Handle rotation input (Q/Left: left, D/Right: right OR Joystick)
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Q) ||
        sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left) ||
        joyLeft) {
        mSprite.rotate(sf::degrees(-Config::CAR_MAX_TURN_RATE * dt * turnFactor));
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D) ||
        sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right) ||
        joyRight) {
        mSprite.rotate(sf::degrees(Config::CAR_MAX_TURN_RATE * dt * turnFactor));
    }

    /// Compute forward direction from rotation
    float angleDeg = mSprite.getRotation().asDegrees();
    float angleRad = angleDeg * 3.14159265359f / 180.f;
    sf::Vector2f forward(std::cos(angleRad), std::sin(angleRad));

    /// Adjust acceleration based on terrain
    bool onGrass = mask.isOnGrass(mSprite.getPosition());
    float accel = onGrass ? Config::CAR_ACCELERATION * 0.4f : Config::CAR_ACCELERATION;

    /// Handle acceleration (Z/Up: forward, S/Down: brake OR Buttons)
    // Bouton 0 (A) pour accélérer, Bouton 1 (B) ou 2 (X) pour freiner
    bool btnAccel = sf::Joystick::isButtonPressed(joystickId, 0);
    bool btnBrake = sf::Joystick::isButtonPressed(joystickId, 1) || sf::Joystick::isButtonPressed(joystickId, 2);

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Z) ||
        sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up) ||
        btnAccel) {
        mVelocity += forward * accel * dt;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S) ||
        sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down) ||
        btnBrake) {
        mVelocity -= forward * Config::CAR_BRAKING * dt;
    }
    // ------------------------------------------------

    /// Update speed after acceleration
    speed = std::sqrt(mVelocity.x * mVelocity.x + mVelocity.y * mVelocity.y);

    /// Limit speed on grass
    if (onGrass && speed > Config::CAR_MAX_SPEED_GRASS) {
        mVelocity *= (1.f - dt * 0.5f); ///< Progressive slowdown
        speed = std::sqrt(mVelocity.x * mVelocity.x + mVelocity.y * mVelocity.y);
        if (speed < Config::CAR_MAX_SPEED_GRASS + 1.f) {
            mVelocity = (mVelocity / speed) * Config::CAR_MAX_SPEED_GRASS;
        }
    }

    /// Apply friction based on input and terrain
    if (speed > 0.1f) {
        // Check input on both sets of keys for friction logic
        bool isAccelerating = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Z) ||
                              sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up) ||
                              btnAccel;

        float friction = isAccelerating ? Config::CAR_FRICTION * 0.5f : Config::CAR_FRICTION;
        if (onGrass) {
            friction *= 0.6f;
        }
        mVelocity -= (mVelocity / speed) * friction * dt;
    }

    /// Limit speed on road
    if (!onGrass && speed > Config::CAR_MAX_SPEED) {
        mVelocity = (mVelocity / speed) * Config::CAR_MAX_SPEED;
    }

    /// Simulate drift
    float forwardSpeed = mVelocity.x * forward.x + mVelocity.y * forward.y;
    sf::Vector2f lateral = mVelocity - forward * forwardSpeed;
    float drift = 0.7f - std::min(speed / 100.f, 0.4f);
    mVelocity = forward * forwardSpeed + lateral * drift;

    /// Update position
    sf::Vector2f newPos = mSprite.getPosition() + mVelocity * dt;

    /// Constrain position within track bounds
    sf::FloatRect bounds = mSprite.getGlobalBounds();
    float w = bounds.size.x / 2.f;
    float h = bounds.size.y / 2.f;
    newPos.x = std::max(trackBounds.position.x + w, std::min(newPos.x, trackBounds.position.x + trackBounds.size.x - w));
    newPos.y = std::max(trackBounds.position.y + h, std::min(newPos.y, trackBounds.position.y + trackBounds.size.y - h));

    /// Stop movement if hitting non-traversable area
    if (!mask.isTraversable(newPos)) {
        mVelocity = {0.f, 0.f};
        return;
    }

    mSprite.setPosition(newPos);
}

/// @brief Render car to window
/// @param window Render target
void Car::render(sf::RenderWindow& window) {
    window.draw(mSprite);
}

/// @brief Get current position
/// @return Position vector
sf::Vector2f Car::getPosition() const {
    return mSprite.getPosition();
}

/// @brief Set new position
/// @param pos New position vector
void Car::setPosition(const sf::Vector2f& pos) {
    mSprite.setPosition(pos);
}

/// @brief Get car sprite
/// @return Reference to sprite
const sf::Sprite& Car::getSprite() const {
    return mSprite;
}

/// @brief Reset velocity to zero
void Car::resetVelocity() {
    mVelocity = {0.f, 0.f};
}

/// @brief Set rotation angle
/// @param angle Angle in degrees
void Car::setRotation(float angle) {
    mSprite.setRotation(sf::degrees(angle));
}

/// @brief Get rotation angle
/// @return Angle in degrees
float Car::getRotation() const {
    return mSprite.getRotation().asDegrees();
}

/// @brief Get current speed
/// @return Speed magnitude
float Car::getSpeed() const {
    return std::sqrt(mVelocity.x * mVelocity.x + mVelocity.y * mVelocity.y); // No SFML alternative for vector norm
}
