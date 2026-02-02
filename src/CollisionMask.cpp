#include "CollisionMask.h"

CollisionMask::CollisionMask() : mScale(1.0f) {}

bool CollisionMask::loadFromFile(const std::string& path) {
    return mImage.loadFromFile(path);
}

// C'est ici qu'on définit le ratio Monde -> Image
void CollisionMask::setScale(float scale) {
    // Si le monde est à l'échelle 0.5, pour retrouver le pixel, on divise par 0.5 (ou multiplie par 1/0.5)
    // mScale stocke le facteur de conversion : PixelImage / PixelMonde
    if (scale > 0.f)
        mScale = 1.0f / scale;
    else
        mScale = 1.0f;
}

sf::Vector2u CollisionMask::worldToImage(sf::Vector2f pos) const {
    // On convertit la position du jeu vers la position sur l'image
    return sf::Vector2u(
        static_cast<unsigned int>(pos.x * mScale),
        static_cast<unsigned int>(pos.y * mScale)
    );
}

// -- Méthodes de détection (inchangées sauf l'appel à worldToImage) --

bool CollisionMask::isOnGrass(sf::Vector2f worldPos) const {
    sf::Vector2u p = worldToImage(worldPos);
    if (p.x >= mImage.getSize().x || p.y >= mImage.getSize().y) return true;
    return mImage.getPixel(p) == sf::Color::Yellow; // Adaptez la couleur (Jaune ou Vert ?)
}

bool CollisionMask::isOnGreen(sf::Vector2f worldPos) const {
    sf::Vector2u p = worldToImage(worldPos);
    if (p.x >= mImage.getSize().x || p.y >= mImage.getSize().y) return false;
    return mImage.getPixel(p) == sf::Color::Green;
}

bool CollisionMask::isOnBlue(sf::Vector2f worldPos) const {
    sf::Vector2u p = worldToImage(worldPos);
    if (p.x >= mImage.getSize().x || p.y >= mImage.getSize().y) return false;
    return mImage.getPixel(p) == sf::Color::Blue;
}

bool CollisionMask::isTraversable(sf::Vector2f worldPos) const {
    sf::Vector2u p = worldToImage(worldPos);
    if (p.x >= mImage.getSize().x || p.y >= mImage.getSize().y) return false;
    return mImage.getPixel(p) != sf::Color::Black;
}