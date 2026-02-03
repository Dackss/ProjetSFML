#ifndef TRACK_H
#define TRACK_H

#include <SFML/Graphics.hpp>
// #include "Entity.h" <-- Supprimé

// Track n'hérite plus de Entity
class Track {
public:
    explicit Track(sf::Texture& texture);

    // Retrait de 'override' car Entity n'existe plus
    void render(sf::RenderWindow& window);

    sf::FloatRect getGlobalBounds() const;

    void setScale(float scale);

private:
    sf::Sprite mSprite;
};

#endif // TRACK_H