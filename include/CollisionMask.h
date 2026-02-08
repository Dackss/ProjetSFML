#ifndef COLLISIONMASK_H
#define COLLISIONMASK_H

#include <SFML/Graphics.hpp>
#include <string>
#include <vector>
#include <cstdint>

// Types de terrain simplifiés pour l'optimisation
enum class TerrainType : uint8_t {
    ROAD,
    WALL,
    GRASS,
    CHECKPOINT,
    FINISH_LINE
};

class CollisionMask {
public:
    CollisionMask();

    bool loadFromFile(const std::string& path);
    void setScale(float scale);

    bool isOnGrass(sf::Vector2f worldPos) const;
    bool isOnGreen(sf::Vector2f worldPos) const; // Checkpoint
    bool isOnBlue(sf::Vector2f worldPos) const;  // Finish
    bool isTraversable(sf::Vector2f worldPos) const;

private:
    sf::Vector2u worldToImage(sf::Vector2f pos) const;
    TerrainType getTerrainAt(unsigned int x, unsigned int y) const;

private:
    sf::Image mImage;
    sf::Vector2u mSize;
    float mScale;

    std::vector<TerrainType> mGrid;
};

#endif