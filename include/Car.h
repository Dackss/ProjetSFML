#ifndef CAR_H
#define CAR_H

#include <SFML/Graphics.hpp>
#include "Entity.h"
#include "CollisionMask.h"

/// @brief Car entity with physics and rendering
class Car : public Entity {
public:
    /// @brief Constructor with texture
    /// @param texture Sprite texture
    explicit Car(sf::Texture& texture);

    /// @brief Update car physics and position
    /// @param deltaTime Time since last update
    /// @param trackBounds Circuit boundaries
    /// @param mask Collision mask
    void update(sf::Time deltaTime, const sf::FloatRect& trackBounds, const CollisionMask& mask);

    /// @brief Render car to window
    /// @param window Render target
    void render(sf::RenderWindow& window) override;

    /// @brief Get current position
    /// @return Position vector
    sf::Vector2f getPosition() const override;

    /// @brief Set new position
    /// @param pos New position vector
    void setPosition(const sf::Vector2f& pos) override;

    /// @brief Get car sprite
    /// @return Reference to sprite
    const sf::Sprite& getSprite() const;

    /// @brief Reset velocity to zero
    void resetVelocity();

    /// @brief Set rotation angle
    /// @param angle Angle in degrees
    void setRotation(float angle);

    /// @brief Get rotation angle
    /// @return Angle in degrees
    float getRotation() const;

    /// @brief Get current speed
    /// @return Speed magnitude
    float getSpeed() const;

private:
    sf::Sprite mSprite;      ///< Car sprite
    sf::Vector2f mVelocity;  ///< Velocity vector
};

#endif // CAR_H