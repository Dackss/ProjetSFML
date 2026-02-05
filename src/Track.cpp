#include "Track.h"

Track::Track(sf::Texture& texture) : mSprite(texture) {
    // Optimisation : Si la texture du circuit n'a pas de pixels semi-transparents,
    // on désactive le lissage s'il n'est pas nécessaire (Pixel Art) ou on le laisse selon Config.
    // mSprite.setTextureRect(...) peut être utilisé si besoin.
}

void Track::render(sf::RenderWindow& window) {
    // Le circuit est le fond du jeu. Il est généralement opaque.
    // Désactiver le blending (calcul de transparence) économise beaucoup de GPU.
    sf::RenderStates states = sf::RenderStates::Default;
    states.blendMode = sf::BlendNone;

    // Note : Si votre PNG a des zones transparentes où l'on doit voir la couleur de fond
    // (le gris défini dans Engine.cpp), retirez la ligne ci-dessus.
    // Mais pour un circuit plein, BlendNone est le meilleur boost de perf.

    window.draw(mSprite, states);
}

sf::FloatRect Track::getGlobalBounds() const {
    return mSprite.getGlobalBounds();
}

void Track::setScale(float scale) {
    mSprite.setScale({scale, scale});
}