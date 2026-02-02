#include "Track.h"
#include <algorithm>

/// @brief Constructor
/// @param texture Track texture
/// @param windowSize Window size for scaling
Track::Track(sf::Texture& texture, const sf::Vector2u& windowSize)
        : mSprite(texture) {
    /// Get texture and window dimensions
    sf::Vector2u textureSize = texture.getSize();
    float windowWidth = windowSize.x;
    float windowHeight = windowSize.y;

    /// Calculate scale to cover window
    float scale = std::max(windowWidth / textureSize.x, windowHeight / textureSize.y);
    mSprite.setScale({scale, scale});

    /// Center sprite in window
    float scaledWidth = textureSize.x * scale;
    float scaledHeight = textureSize.y * scale;
    float posX = scaledWidth >= windowWidth ? 0.f : (windowWidth - scaledWidth) / 2.f;
    float posY = scaledHeight >= windowHeight ? 0.f : (windowHeight - scaledHeight) / 2.f;

    /// Set sprite position
    /// @see https://www.sfml-dev.org/documentation/3.0.0/classsf_1_1Transformable.php#a4dbfb1a7c80688b0b4c4d6b2b0e6a04b
    mSprite.setPosition({posX, posY});
}

/// @brief Render track
/// @param window Render target
/// @see https://www.sfml-dev.org/documentation/3.0.0/classsf_1_1RenderWindow.php#a839bbf336bd120d2a91d87a47f5296b2
void Track::render(sf::RenderWindow& window) {
    window.draw(mSprite);
}