#ifndef TRACK_H
#define TRACK_H

#include <SFML/Graphics.hpp>
#include "Entity.h"

class Track : public Entity {
public:
    explicit Track(sf::Texture& texture);
    void render(sf::RenderWindow& window) override;
    sf::FloatRect getGlobalBounds() const;

    // Ajout de la méthode pour redimensionner proprement
    void setScale(float scale);

private:
    sf::Sprite mSprite;
};

#endif // TRACK_H