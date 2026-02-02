#ifndef TRACK_H
#define TRACK_H

#include <SFML/Graphics.hpp>
#include "Entity.h"

/// @brief Represents the game track
class Track : public Entity {
public:
    /// @brief Constructor
    /// @param texture Track texture
    /// @param windowSize Window size for scaling
    explicit Track(sf::Texture& texture, const sf::Vector2u& windowSize);

    /// @brief Render track
    /// @param window Render target
    void render(sf::RenderWindow& window) override;

private:
    sf::Sprite mSprite; ///< Track sprite
};

#endif // TRACK_H