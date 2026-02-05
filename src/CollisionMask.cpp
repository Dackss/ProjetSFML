#include "CollisionMask.h"

CollisionMask::CollisionMask() : mScale(1.0f) {}

bool CollisionMask::loadFromFile(const std::string& path) {
    if (!mImage.loadFromFile(path)) return false;

    // Initialisation du cache
    // getPixelsPtr() renvoie un const std::uint8_t* en SFML 3
    mPixelData = mImage.getPixelsPtr();
    mSize = mImage.getSize();
    return true;
}

void CollisionMask::setScale(float scale) {
    if (scale > 0.f)
        mScale = 1.0f / scale;
    else
        mScale = 1.0f;
}

sf::Vector2u CollisionMask::worldToImage(sf::Vector2f pos) const {
    return sf::Vector2u(
        static_cast<unsigned int>(pos.x * mScale),
        static_cast<unsigned int>(pos.y * mScale)
    );
}

// Helper optimisé avec std::uint8_t
inline bool isColorMatch(const std::uint8_t* pixels, unsigned int x, unsigned int y, unsigned int width,
                         std::uint8_t r, std::uint8_t g, std::uint8_t b) {
    unsigned int index = (x + y * width) * 4;
    return pixels[index] == r && pixels[index + 1] == g && pixels[index + 2] == b;
}

bool CollisionMask::isOnGrass(sf::Vector2f worldPos) const {
    sf::Vector2u p = worldToImage(worldPos);
    if (p.x >= mSize.x || p.y >= mSize.y) return true;

    // Vérifiez si votre masque utilise le Jaune ou le Vert pour l'herbe
    // Ici : Jaune (255, 255, 0)
    return isColorMatch(mPixelData, p.x, p.y, mSize.x, 255, 255, 0);
}

bool CollisionMask::isOnGreen(sf::Vector2f worldPos) const {
    sf::Vector2u p = worldToImage(worldPos);
    if (p.x >= mSize.x || p.y >= mSize.y) return false;
    // Vert : Checkpoint (0, 255, 0)
    return isColorMatch(mPixelData, p.x, p.y, mSize.x, 0, 255, 0);
}

bool CollisionMask::isOnBlue(sf::Vector2f worldPos) const {
    sf::Vector2u p = worldToImage(worldPos);
    if (p.x >= mSize.x || p.y >= mSize.y) return false;
    // Bleu : Ligne d'arrivée (0, 0, 255)
    return isColorMatch(mPixelData, p.x, p.y, mSize.x, 0, 0, 255);
}

bool CollisionMask::isTraversable(sf::Vector2f worldPos) const {
    sf::Vector2u p = worldToImage(worldPos);
    if (p.x >= mSize.x || p.y >= mSize.y) return false;

    // Vérifie que ce n'est PAS Noir (0, 0, 0)
    unsigned int index = (p.x + p.y * mSize.x) * 4;
    return !(mPixelData[index] == 0 && mPixelData[index+1] == 0 && mPixelData[index+2] == 0);
}