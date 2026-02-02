#ifndef COLLISIONMASK_H
#define COLLISIONMASK_H

#include <SFML/Graphics.hpp>

/// @brief Detects special zones by color in an image
class CollisionMask {
public:
    /// @brief Load mask image
    /// @param path File path
    /// @return True if loaded successfully
    bool loadFromFile(const std::string& path);

    /// @brief Set scale factor for coordinates
    /// @param scale Scale factor
    void setScale(float scale);

    /// @brief Check if position is on grass (yellow)
    /// @param worldPos World coordinates
    /// @return True if on grass
    bool isOnGrass(sf::Vector2f worldPos) const;

    /// @brief Check if position is on checkpoint (green)
    /// @param worldPos World coordinates
    /// @return True if on checkpoint
    bool isOnGreen(sf::Vector2f worldPos) const;

    /// @brief Check if position is on finish line (blue)
    /// @param worldPos World coordinates
    /// @return True if on finish line
    bool isOnBlue(sf::Vector2f worldPos) const;

    /// @brief Check if position is traversable (not black)
    /// @param worldPos World coordinates
    /// @return True if traversable
    bool isTraversable(sf::Vector2f worldPos) const;

private:
    sf::Image mImage; ///< Mask image
    float mScale;     ///< Scale factor for coordinates
};

#endif // COLLISIONMASK_H