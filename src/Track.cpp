#include "Track.h"

Track::Track(sf::Texture& texture) : mSprite(texture) {
    // On ne décide plus de l'échelle ici.
    // On laisse World.cpp la configurer.
}

void Track::render(sf::RenderWindow& window) {
    window.draw(mSprite);
}

sf::FloatRect Track::getGlobalBounds() const {
    return mSprite.getGlobalBounds();
}

// Nouvelle méthode pour forcer la bonne taille
void Track::setScale(float scale) {
    mSprite.setScale({scale, scale});
}