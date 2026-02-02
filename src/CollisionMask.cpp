#include "CollisionMask.h"
#include <cmath>

/// @brief Load mask image
/// @param path File path
/// @return True if loaded successfully
bool CollisionMask::loadFromFile(const std::string& path) {
    return mImage.loadFromFile(path);
}

/// @brief Set scale factor for coordinates
/// @param scale Scale factor
void CollisionMask::setScale(float scale) {
    mScale = scale;
}

/// @brief Get pixel color at world position
/// @param worldPos World coordinates
/// @param pixel Output pixel color
/// @return True if position is valid
bool getPixelAt(const sf::Image& image, float scale, sf::Vector2f worldPos, sf::Color& pixel) {
    int x = static_cast<int>(worldPos.x / scale);
    int y = static_cast<int>(worldPos.y / scale);

    int imgW = static_cast<int>(image.getSize().x);
    int imgH = static_cast<int>(image.getSize().y);

    if (x < 0 || y < 0 || x >= imgW || y >= imgH) {
        return false;
    }

    pixel = image.getPixel({(unsigned)x, (unsigned)y});
    return true;
}

/// @brief Check if position is on grass (yellow)
/// @param worldPos World coordinates
/// @return True if on grass
bool CollisionMask::isOnGrass(sf::Vector2f worldPos) const {
    sf::Color pixel;
    if (!getPixelAt(mImage, mScale, worldPos, pixel)) {
        return false;
    }

    /// Compare pixel to yellow (255, 255, 0) with tolerance
    int diffR = std::abs(int(pixel.r) - 255);
    int diffG = std::abs(int(pixel.g) - 255);
    int diffB = std::abs(int(pixel.b) - 0);
    return diffR <= 50 && diffG <= 50 && diffB <= 50;
}

/// @brief Check if position is on checkpoint (green)
/// @param worldPos World coordinates
/// @return True if on checkpoint
bool CollisionMask::isOnGreen(sf::Vector2f worldPos) const {
    sf::Color pixel;
    if (!getPixelAt(mImage, mScale, worldPos, pixel)) {
        return false;
    }

    /// Compare pixel to green (0, 255, 0) with tolerance
    int diffR = std::abs(int(pixel.r) - 0);
    int diffG = std::abs(int(pixel.g) - 255);
    int diffB = std::abs(int(pixel.b) - 0);
    return diffR <= 60 && diffG <= 60 && diffB <= 60;
}

/// @brief Check if position is on finish line (blue)
/// @param worldPos World coordinates
/// @return True if on finish line
bool CollisionMask::isOnBlue(sf::Vector2f worldPos) const {
    sf::Color pixel;
    if (!getPixelAt(mImage, mScale, worldPos, pixel)) {
        return false;
    }

    /// Compare pixel to blue (0, 0, 255) with tolerance
    int diffR = std::abs(int(pixel.r) - 0);
    int diffG = std::abs(int(pixel.g) - 0);
    int diffB = std::abs(int(pixel.b) - 255);
    return diffR <= 50 && diffG <= 50 && diffB <= 50;
}

/// @brief Check if position is traversable (not black)
/// @param worldPos World coordinates
/// @return True if traversable
bool CollisionMask::isTraversable(sf::Vector2f worldPos) const {
    sf::Color pixel;
    if (!getPixelAt(mImage, mScale, worldPos, pixel)) {
        return false;
    }

    /// Consider pixel traversable if not black (threshold 20)
    return pixel.r > 20 || pixel.g > 20 || pixel.b > 20;
}