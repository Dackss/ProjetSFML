#include "CollisionMask.h"

CollisionMask::CollisionMask() : mScale(1.0f) {}

bool CollisionMask::loadFromFile(const std::string& path) {
    if (!mImage.loadFromFile(path)) return false;

    mSize = mImage.getSize();
    const std::uint8_t* pixels = mImage.getPixelsPtr();

    // Initialisation de la grille
    mGrid.resize(mSize.x * mSize.y);

    // Pré-calcul complet de la carte
    for (unsigned int y = 0; y < mSize.y; ++y) {
        for (unsigned int x = 0; x < mSize.x; ++x) {
            unsigned int i = (x + y * mSize.x) * 4;
            std::uint8_t r = pixels[i];
            std::uint8_t g = pixels[i + 1];
            std::uint8_t b = pixels[i + 2];
            // a = pixels[i+3] (non utilisé)

            TerrainType type = TerrainType::ROAD; // Par défaut

            // Logique de détection (ordre de priorité)
            if (r == 0 && g == 0 && b == 0) {
                type = TerrainType::WALL;
            }
            else if (r == 255 && g == 255 && b == 0) { // Jaune
                type = TerrainType::GRASS;
            }
            else if (r == 0 && g == 255 && b == 0) {   // Vert
                type = TerrainType::CHECKPOINT;
            }
            else if (r == 0 && g == 0 && b == 255) {   // Bleu
                type = TerrainType::FINISH_LINE;
            }

            // Stockage linéaire
            mGrid[x + y * mSize.x] = type;
        }
    }

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

// Méthode helper interne sécurisée
TerrainType CollisionMask::getTerrainAt(unsigned int x, unsigned int y) const {
    if (x >= mSize.x || y >= mSize.y) return TerrainType::GRASS; // Hors map = Herbe (ou Mur selon choix)
    return mGrid[x + y * mSize.x];
}

// --- ACCESSEURS OPTIMISÉS (Lecture directe RAM) ---

bool CollisionMask::isOnGrass(sf::Vector2f worldPos) const {
    sf::Vector2u p = worldToImage(worldPos);
    // Si hors map, c'est de l'herbe, sinon check la grille
    return getTerrainAt(p.x, p.y) == TerrainType::GRASS;
}

bool CollisionMask::isOnGreen(sf::Vector2f worldPos) const {
    sf::Vector2u p = worldToImage(worldPos);
    return getTerrainAt(p.x, p.y) == TerrainType::CHECKPOINT;
}

bool CollisionMask::isOnBlue(sf::Vector2f worldPos) const {
    sf::Vector2u p = worldToImage(worldPos);
    return getTerrainAt(p.x, p.y) == TerrainType::FINISH_LINE;
}

bool CollisionMask::isTraversable(sf::Vector2f worldPos) const {
    sf::Vector2u p = worldToImage(worldPos);
    TerrainType t = getTerrainAt(p.x, p.y);
    // Traversable si ce n'est pas un MUR
    return t != TerrainType::WALL;
}